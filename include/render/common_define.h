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
    struct RenderGlobalResourceInfo;

    extern std::shared_ptr<VulkanAPI::VulkanContext> g_p_vulkan_context;

    struct DirectionLightInfo
    {
        VkDeviceSize shadowmap_width = 1000;
        VkDeviceSize shadowmap_height = 1000;
        float camera_width = 150;
        float camera_height = 150;
        float camera_near = 1.0f;
        float camera_far = 100.0f;
        VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    };

    struct ImageAttachment
    {
        VkImage            image{VK_NULL_HANDLE};
        VkDeviceMemory     mem{VK_NULL_HANDLE};
        VkImageView        view{VK_NULL_HANDLE};
        VkFormat           format{VK_FORMAT_UNDEFINED};
        VkImageLayout      layout{VK_IMAGE_LAYOUT_UNDEFINED}; // 使用图像之前，需要将其转换为适当的布局
        VkImageUsageFlags  usage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
        VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
        VkImageViewType    view_type{VK_IMAGE_VIEW_TYPE_2D};
        VkDeviceSize       width{0}, height{0};
        uint32_t           layer_count{1};


        void init()
        {
            assert(g_p_vulkan_context != nullptr);
            VulkanAPI::VulkanUtil::createImage(g_p_vulkan_context,
                                               width,
                                               height,
                                               format,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               usage,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               image,
                                               mem,
                                               0,
                                               layer_count,
                                               1);

            view = VulkanAPI::VulkanUtil::createImageView(g_p_vulkan_context,
                                                          image,
                                                          format,
                                                          aspect,
                                                          view_type,
                                                          1,
                                                          0,
                                                          layer_count);

        }

        void destroy()
        {
            assert(g_p_vulkan_context != nullptr);
            vkDestroyImageView(g_p_vulkan_context->_device, view, nullptr);
            vkDestroyImage(g_p_vulkan_context->_device, image, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, mem, nullptr);
            format = VK_FORMAT_UNDEFINED;
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
            width  = 0;
            height = 0;
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
