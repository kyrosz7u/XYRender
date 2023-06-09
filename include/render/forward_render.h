//
// Created by kyrosz7u on 2023/5/19.
//

#ifndef XEXAMPLE_FORWARD_RENDER_H
#define XEXAMPLE_FORWARD_RENDER_H

#include "render_base.h"
#include "renderpass/main_camera_renderpass.h"
#include "render/resource/render_ubo.h"


namespace RenderSystem
{
    class ForwardRender : public RenderBase
    {
    public:
        enum _renderpasses
        {
            _main_camera_renderpass = 0,
            _ui_overlay_renderpass,
            _renderpass_count
        };

        ~ForwardRender() override;

        void initialize() override;

        void setupRenderTargets();

        void setupBackupBuffer();

        void setViewport();

        void Tick() override;

        void draw();

        void setCameraMatrix(Matrix4x4 view, Matrix4x4 proj)
        {
            m_view_matrix = view;
            m_proj_matrix = proj;
        }

        void setUIOverlay(UIOverlayPtr ui_overlay)
        {
            m_p_ui_overlay = ui_overlay;
        }

        void SetVisibleRenderData(std::vector<RenderSubmesh> *p_visible_submesh,
                                  std::vector<Texture2DPtr> *p_visible_texture);

        void UpdateRenderModelUBOList(std::vector<VulkanModelDefine> &model_matrix);

        void UpdateRenderPerFrameScenceUBO(VulkanPerFrameSceneDefine &per_frame_scene_define);

    private:

        void setupCommandBuffer();

        void setupDescriptorPool();

        void setupRenderpass();

        void updateAfterSwapchainRecreate();

    private:
        VkCommandPool                m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;
        VkDescriptorPool             m_descriptor_pool;
        std::vector<RenderPassPtr>   m_render_passes;
        // render target
        std::vector<ImageAttachment> m_render_targets;
        std::vector<ImageAttachment> m_backup_targets;
        // ubo
        RenderModelUBOList           m_render_model_ubo_list;
        RenderPerFrameUBO m_render_per_frame_ubo;
        // texture descriptor list
        std::vector<VkDescriptorSet> m_texture_descriptor_sets;

        Matrix4x4         m_view_matrix;
        Matrix4x4         m_proj_matrix;

        UIOverlayPtr m_p_ui_overlay;
    };
}
#endif //XEXAMPLE_FORWARD_RENDER_H
