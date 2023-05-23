//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "vulkan_subpass.h"

namespace VulkanAPI
{
    struct ImageAttachment
    {
        VkImage        image;
        VkDeviceMemory mem;
        VkImageView    view;
        VkFormat       format;
        VkImageLayout  layout;
    };

    struct Framebuffer
    {
        int           width;
        int           height;
        VkFramebuffer framebuffer;

        std::vector<ImageAttachment> attachments;
    };

    class RenderPassInitInfo
    {
    public:
        std::vector<ImageAttachment> renderTargets;
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
        virtual void initialize(RenderPassInitInfo* renderPassInitInfo) = 0;
        virtual void setupSubpass() = 0;
        virtual void draw() = 0;
    protected:
        static std::shared_ptr<VulkanContext> m_p_vulkan_context;
        std::vector<std::shared_ptr<VulkanSubPassBase>> m_subpass_list;
    };
}

#endif //VULKANRENDER_VULKAN_RENDERPASS_H
