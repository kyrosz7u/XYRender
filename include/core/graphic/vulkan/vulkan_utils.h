//
// Created by kyrosz7u on 4/6/23.
//

#ifndef VULKANRENDER_VULKAN_UTILS_H
#define VULKANRENDER_VULKAN_UTILS_H

#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include "vulkan_context.h"

#include "vulkan/vulkan.h"

#include <iostream>
#include <unordered_map>
#include <vector>

namespace VulkanAPI
{
    class VulkanUtil
    {
    public:
        static uint32_t
        findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag);

        static std::string errorString(VkResult errorCode);

        static std::string physicalDeviceTypeString(VkPhysicalDeviceType type);

        static VkShaderModule createShaderModule(VkDevice device, const std::vector<unsigned char> &shader_code);

        static void createBuffer(std::shared_ptr<VulkanContext> p_context,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &buffer_memory);

        static void copyBuffer(std::shared_ptr<VulkanContext> p_context,
                               VkBuffer srcBuffer,
                               VkBuffer dstBuffer,
                               VkDeviceSize srcOffset,
                               VkDeviceSize dstOffset,
                               VkDeviceSize size);

        static void createImage(std::shared_ptr<VulkanContext> p_context,
                                uint32_t image_width,
                                uint32_t image_height,
                                VkFormat format,
                                VkImageTiling image_tiling,
                                VkImageUsageFlags image_usage_flags,
                                VkMemoryPropertyFlags memory_property_flags,
                                VkImage &image, VkDeviceMemory &memory,
                                VkImageCreateFlags image_create_flags,
                                uint32_t array_layers,
                                uint32_t miplevels);

        static void createCubeMapImage(std::shared_ptr<VulkanContext> p_context,
                                        uint32_t image_width,
                                        uint32_t image_height,
                                        VkFormat image_format,
                                        VkMemoryPropertyFlags memory_property_flags,
                                        VkImage &image,
                                        VkDeviceMemory &memory,
                                        uint32_t mip_levels);

        static VkImageView createImageView(std::shared_ptr<VulkanContext> p_context,
                                           VkImage &image,
                                           VkFormat format,
                                           VkImageAspectFlags image_aspect_flags,
                                           VkImageViewType view_type,
                                           uint32_t mip_levels,
                                           uint32_t base_layer=0,
                                           uint32_t layout_count=1);

        static VkImageView createImageView(VkDevice device,
                                           VkImage &image,
                                           VkFormat format,
                                           VkImageAspectFlags image_aspect_flags,
                                           VkImageViewType view_type,
                                           uint32_t layout_count,
                                           uint32_t miplevels);

        static void transitionImageLayout(std::shared_ptr<VulkanContext> p_context,
                                          VkImage image,
                                          VkImageLayout old_layout,
                                          VkImageLayout new_layout,
                                          uint32_t layer_count,
                                          uint32_t miplevels,
                                          VkImageAspectFlags aspect_mask_bits,
                                          VkCommandBuffer command_buffer = VK_NULL_HANDLE);

        static void copyBufferToImage(std::shared_ptr<VulkanContext> p_context,
                                      VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t layer_count);

        static void genMipmappedImage(std::shared_ptr<VulkanContext> p_context,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t mip_levels);

        static VkSampler getOrCreateMipmapSampler(std::shared_ptr<VulkanContext> p_context,
                                                  uint32_t mip_levels);

        static VkSampler getOrCreateNearestSampler(std::shared_ptr<VulkanContext> p_context);

        static VkSampler getOrCreateLinearSampler(std::shared_ptr<VulkanContext> p_context);

        static VkSampler getOrCreateDepthSampler(std::shared_ptr<VulkanContext> p_context);

        static VkSampler getOrCreateCubeMapSampler(std::shared_ptr<VulkanContext> p_context, uint32_t mip_levels);

        static void destroyMipmappedSampler(VkDevice device);

        static void destroyNearestSampler(VkDevice device);

        static void destroyLinearSampler(VkDevice device);

    private:
        static std::unordered_map<uint32_t, VkSampler> m_mipmap_sampler_texture;
        static VkSampler                               m_nearest_sampler;
        static VkSampler                               m_linear_sampler;
        static VkSampler                               m_depth_sampler;
        static VkSampler                               m_cubemap_sampler;

    };
} // namespace Pilot

#endif //VULKANRENDER_VULKAN_UTILS_H

