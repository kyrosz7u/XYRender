//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "vulkan_subpass.h"

namespace VulkanAPI
{
    struct ImageAttachment;
    struct Framebuffer;
    struct RenderPassInitInfo
    {
        RenderCommandInfo* render_command_info;
        // [target_index][render_image_index]
        std::vector<std::vector<ImageAttachment>> render_targets;

        std::vector<VkClearValue> clearValues;
//        std::vector<VkSubpassDependency> subpassDependencies;
//        std::vector<VkSubpassDescription> subpassDescriptions;
    };

    class RenderPassBase
    {
    public:
        RenderPassBase()
        {
            assert(m_p_vulkan_context!= nullptr);
        }
        
        static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
        {
            m_p_vulkan_context = vulkanContext;
        }

        virtual void initialize(RenderPassInitInfo* renderpass_init_info) = 0;
        virtual void draw(int render_image_index) = 0;
        virtual void updateAfterSwapchainRecreate() = 0;
    protected:
        friend struct ImageAttachment;
        virtual void setupSubpass() = 0;
        static std::shared_ptr<VulkanContext> m_p_vulkan_context;
        RenderCommandInfo* m_p_render_command_info;
        std::vector<std::vector<ImageAttachment>> m_render_targets; // [target_index][render_image_index]
        std::vector<std::shared_ptr<VulkanSubPassBase>> m_subpass_list;
    };

    struct ImageAttachment
    {
        VkImage        image;
        VkDeviceMemory mem;
        VkImageView    view;
        VkFormat       format;
        VkImageLayout  layout; // 使用图像之前，需要将其转换为适当的布局
        void destroy()
        {
            assert(RenderPassBase::m_p_vulkan_context!= nullptr);
            vkDestroyImageView(RenderPassBase::m_p_vulkan_context->_device, view, nullptr);
            vkDestroyImage(RenderPassBase::m_p_vulkan_context->_device, image, nullptr);
            vkFreeMemory(RenderPassBase::m_p_vulkan_context->_device, mem, nullptr);
        }
    };

    struct Framebuffer
    {
        int           width;
        int           height;
        VkFramebuffer framebuffer;

        std::vector<ImageAttachment> attachments;
    };
}

#endif //VULKANRENDER_VULKAN_RENDERPASS_H
