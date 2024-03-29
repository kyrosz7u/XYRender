//
// Created by kyrosz7u on 2023/6/14.
//

#include "renderpass_base.h"
#include "render/shadow.h"

#ifndef XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
#define XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
namespace RenderSystem
{
    struct DirectionalLightShadowRenderPassInitInfo : public RenderPassInitInfo
    {
        ImageAttachment *shadowmap_attachment;
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
            for (int i = 0; i < m_framebuffer_per_rendertarget.size(); i++)
                vkDestroyFramebuffer(g_p_vulkan_context->_device, m_framebuffer_per_rendertarget[i], nullptr);
            for (int i = 0; i < m_renderpass_attachments.size(); i++)
                vkDestroyImageView(g_p_vulkan_context->_device, m_renderpass_attachments[i].view, nullptr);
            vkDestroyRenderPass(g_p_vulkan_context->_device, m_renderpass, nullptr);
        }

        void initialize(RenderPassInitInfo *renderpass_init_info) override;

        void updateAfterSwapchainRecreate() override;

        void SetShadowData(const std::vector<DirLightShadow> &dir_light_shadow_list);

        void draw(uint32_t render_target_index) override;

        void drawMultiThreading(uint32_t render_target_index, uint32_t command_buffer_index) override;

    private:
        ImageAttachment             *m_p_shadowmap_attachment;
        const std::vector<DirLightShadow> *m_dir_light_shadow_list;

        void setupRenderpassAttachments();

        void setupRenderPass();

        void setupFrameBuffer();

        void setupSubpass() override;

        void clearShadowMap(VkCommandBuffer &command_buffer, VkImage &image);
    };
}
#endif //XEXAMPLE_DIRECTION_LIGHT_SHADOW_RENDERPASS_H
