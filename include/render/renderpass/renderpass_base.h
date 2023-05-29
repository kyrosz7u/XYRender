//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "render/subpass/subpass_base.h"
#include "render/render_base.h"

namespace RenderSystem
{
    struct ImageAttachment;
    struct Framebuffer;

    struct RenderPassInitInfo
    {
        RenderCommandInfo* render_command_info = nullptr;

        RenderGlobalResourceInfo* render_resource_info;

        std::vector<ImageAttachment>* render_targets = nullptr;

        VkDescriptorPool* descriptor_pool = nullptr;

        std::vector<VkClearValue> clearValues;
//        std::vector<VkSubpassDependency> subpassDependencies;
//        std::vector<VkSubpassDescription> subpassDescriptions;
    };

    class RenderPassBase
    {
    public:
        RenderPassBase()
        {
            assert(g_p_vulkan_context != nullptr);
        }
        
        static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
        {
            g_p_vulkan_context = vulkanContext;
        }

        virtual void initialize(RenderPassInitInfo* renderpass_init_info) = 0;
        virtual void draw(int render_image_index) = 0;
        virtual void updateAfterSwapchainRecreate() = 0;
    protected:
        friend struct ImageAttachment;
        virtual void setupSubpass() = 0;
        RenderCommandInfo* m_p_render_command_info;
        RenderGlobalResourceInfo* m_p_render_resource_info;
        std::vector<ImageAttachment>* m_p_render_targets; // [target_index][render_image_index]
        std::vector<std::shared_ptr<SubPass::SubPassBase>> m_subpass_list;
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
            assert(g_p_vulkan_context != nullptr);
            vkDestroyImageView(g_p_vulkan_context->_device, view, nullptr);
            vkDestroyImage(g_p_vulkan_context->_device, image, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, mem, nullptr);
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
