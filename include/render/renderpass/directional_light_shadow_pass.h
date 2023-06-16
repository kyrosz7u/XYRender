//
// Created by kyrosz7u on 2023/6/14.
//

#include "renderpass_base.h"

#ifndef XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
#define XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
namespace RenderSystem
{
    struct DirectionalLightShadowRenderPassInitInfo : public RenderPassInitInfo
    {

    };

    class DirectionalLightShadowRenderPass : public RenderPassBase
    {
    public:
        enum _direction_light_shadow_subpass : unsigned int
        {
            _direction_light_shadow_subpass_shadow = 0,
            _direction_light_shadow_subpass_count
        };

        enum _direction_light_attachment : unsigned int
        {
            _direction_light_attachment_depth = 0,
            _direction_light_attachment_count
        };

        DirectionalLightShadowRenderPass()
        {
            m_subpass_list.resize(_direction_light_shadow_subpass_count, VK_NULL_HANDLE);
        }

        ~DirectionalLightShadowRenderPass()
        {
            for(int i = 0; i < m_framebuffer_per_rendertarget.size(); i++)
                vkDestroyFramebuffer(g_p_vulkan_context->_device, m_framebuffer_per_rendertarget[i], nullptr);
            vkDestroyRenderPass(g_p_vulkan_context->_device, m_renderpass, nullptr);
        }

        void initialize(RenderPassInitInfo *renderpass_init_info) override;

        void updateAfterSwapchainRecreate();

        void draw(int render_target_index);

    private:
        void setupRenderPass();

        void setupRenderpassAttachments();

        void setupFrameBuffer();

        void setupSubpass() override;
    };
}
#endif //XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
