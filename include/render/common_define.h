//
// Created by kyrosz7u on 2023/6/1.
//

#ifndef XEXAMPLE_COMMON_DEFINE_H
#define XEXAMPLE_COMMON_DEFINE_H

#include <vulkan/vulkan.h>
#include "graphic/vulkan/vulkan_context.h"
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
        VulkanAPI::RenderCommandInfo* render_command_info = nullptr;

        RenderGlobalResourceInfo* render_resource_info;

        std::vector<ImageAttachment>* render_targets = nullptr;

        VkDescriptorPool* descriptor_pool = nullptr;

        std::vector<VkClearValue> clearValues;
//        std::vector<VkSubpassDependency> subpassDependencies;
//        std::vector<VkSubpassDescription> subpassDescriptions;
    };


}
#endif //XEXAMPLE_COMMON_DEFINE_H
