//
// Created by kyrosz7u on 2023/6/1.
//

#ifndef XEXAMPLE_COMMON_DEFINE_H
#define XEXAMPLE_COMMON_DEFINE_H

#include <vulkan/vulkan.h>
#include "core/graphic/vulkan/vulkan_context.h"
#include "core/graphic/vulkan/vulkan_utils.h"
#include <memory>
#include <vector>

namespace RenderSystem
{
    extern std::shared_ptr<VulkanAPI::VulkanContext> g_p_vulkan_context;

    struct RenderGlobalResourceInfo;

    struct ImageAttachment
    {
        VkImage        image;
        VkDeviceMemory mem;
        VkImageView    view;
        VkFormat       format;
        VkImageLayout  layout; // 使用图像之前，需要将其转换为适当的布局
        VkDeviceSize   width, height;

        void init()
        {
            assert(g_p_vulkan_context != nullptr);
            VulkanAPI::VulkanUtil::createImage(g_p_vulkan_context,
                                               width,
                                               height,
                                               format,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               image,
                                               mem,
                                               0,
                                               1,
                                               1);

            view = VulkanAPI::VulkanUtil::createImageView(g_p_vulkan_context,
                                                          image,
                                                          format,
                                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                                          VK_IMAGE_VIEW_TYPE_2D,
                                                          1,
                                                          1);

        }

        void destroy()
        {
            assert(g_p_vulkan_context != nullptr);
            vkDestroyImageView(g_p_vulkan_context->_device, view, nullptr);
            vkDestroyImage(g_p_vulkan_context->_device, image, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, mem, nullptr);
        }
    };

    struct RenderPassInitInfo
    {
        VulkanAPI::RenderCommandInfo *render_command_info = nullptr;

        RenderGlobalResourceInfo *render_resource_info;

        std::vector<ImageAttachment> *render_targets = nullptr;

        VkDescriptorPool *descriptor_pool = nullptr;

        std::vector<VkClearValue> clearValues;
//        std::vector<VkSubpassDependency> subpassDependencies;
//        std::vector<VkSubpassDescription> subpassDescriptions;
    };


}
#endif //XEXAMPLE_COMMON_DEFINE_H
