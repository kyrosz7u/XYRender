//
// Created by kyrosz7u on 2023/6/1.
//

#ifndef EXAMPLEX_UI_OVERLAY_RENDERPASS_H
#define EXAMPLEX_UI_OVERLAY_RENDERPASS_H

#include "renderpass_base.h"

namespace RenderSystem
{
    struct UIOverlayRenderPassInitInfo:public RenderPassInitInfo
    {
        ImageAttachment* in_color_attachment = nullptr;
    };

    class UIOverlayRenderPass : public RenderPassBase
    {
    public:
        enum _ui_overlay_subpass
        {
            _ui_overlay_subpass_ui,
            _ui_overlay_subpass_combine,
            _ui_overlay_subpass_count
        };

        enum _ui_overlay_framebuffer_attachment
        {
            _ui_overlay_framebuffer_attachment_in_color,
            _ui_overlay_framebuffer_attachment_backup_color,
            _ui_overlay_framebuffer_attachment_out_color,
            _ui_overlay_framebuffer_attachment_count
        };
        UIOverlayRenderPass()
        {
            m_subpass_list.resize(_ui_overlay_subpass_count);
            m_renderpass_attachments.resize(_ui_overlay_framebuffer_attachment_count);
        }
        void initialize(RenderPassInitInfo* renderpass_init_info) override;
        void draw(int render_target_index) override;
        void updateAfterSwapchainRecreate() override;
    private:
        void setupRenderPass();
        void setupRenderpassAttachments();
        void setupFrameBuffer();
        void setupSubpass() override;

    private:
        std::vector<ImageAttachment> m_renderpass_attachments;
        std::vector<Framebuffer>   m_framebuffer_per_rendertarget;
        ImageAttachment* m_p_in_color_attachment = nullptr;
    };
}
#endif //EXAMPLEX_UI_OVERLAY_RENDERPASS_H
