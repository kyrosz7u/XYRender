//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "render/common_define.h"
#include "render/subpass/subpass_base.h"

namespace RenderSystem
{
    using namespace VulkanAPI;
    class RenderPassBase;
    typedef std::shared_ptr<RenderPassBase> RenderPassPtr;
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
        VkRenderPass m_renderpass;
        std::vector<ImageAttachment>* m_p_render_targets; // [target_index][render_image_index]
        std::vector<std::shared_ptr<SubPass::SubPassBase>> m_subpass_list;
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
