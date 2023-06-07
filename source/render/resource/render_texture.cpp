//
// Created by kyrosz7u on 2023/6/7.
//

#include "render/resource/render_texture.h"
#include "core/graphic/vulkan/vulkan_utils.h"
#include <cmath>
#include <stdexcept>

using namespace RenderSystem;
using namespace VulkanAPI;

Texture2D::Texture2D(std::string texture_path, bool gen_mipmap)
{
    int          texWidth, texHeight, texChannels;
    stbi_uc      *pixels   = stbi_load(texture_path.c_str(),
                                       &texWidth,
                                       &texHeight,
                                       &texChannels,
                                       STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    width      = texWidth;
    height     = texHeight;
    mip_levels = gen_mipmap ? static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1 : 0;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanUtil::createBuffer(g_p_vulkan_context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(g_p_vulkan_context->_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(g_p_vulkan_context->_device, stagingBufferMemory);

    stbi_image_free(pixels);

    VulkanUtil::createImage(g_p_vulkan_context,
                            texWidth, texHeight,
                            VK_FORMAT_R8G8B8A8_UNORM,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory,
                            0, 0, mip_levels);

    VulkanUtil::transitionImageLayout(g_p_vulkan_context,
                                      image,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      1, mip_levels, 0);

    VulkanUtil::copyBufferToImage(g_p_vulkan_context,
                                  stagingBuffer,
                                  image,
                                  static_cast<uint32_t>(texWidth),
                                  static_cast<uint32_t>(texHeight), 1);

    vkFreeMemory(g_p_vulkan_context->_device, stagingBufferMemory, nullptr);
    vkDestroyBuffer(g_p_vulkan_context->_device, stagingBuffer, nullptr);

    if (gen_mipmap)
    {
        VulkanUtil::genMipmappedImage(g_p_vulkan_context, image, texWidth, texHeight, mip_levels);
    }
    if (gen_mipmap)
    {
        sampler = VulkanUtil::getOrCreateLinearSampler(g_p_vulkan_context);
    } else
    {
        sampler = VulkanUtil::getOrCreateMipmapSampler(g_p_vulkan_context, mip_levels);
    }
    view = VulkanUtil::createImageView(g_p_vulkan_context,
                                       image,
                                       VK_FORMAT_R8G8B8A8_UNORM,
                                       VK_IMAGE_ASPECT_COLOR_BIT,
                                       VK_IMAGE_VIEW_TYPE_2D,
                                       1, mip_levels);

    image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

Texture2D::~Texture2D()
{
    vkDestroyImageView(g_p_vulkan_context->_device, view, nullptr);
    vkDestroyImage(g_p_vulkan_context->_device, image, nullptr);
    vkFreeMemory(g_p_vulkan_context->_device, memory, nullptr);
    image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}
