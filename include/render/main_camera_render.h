//
// Created by kyrosz7u on 2023/5/19.
//

#ifndef XEXAMPLE_MAIN_CAMERA_RENDER_H
#define XEXAMPLE_MAIN_CAMERA_RENDER_H

#include "render_base.h"
#include "renderpass/main_camera_renderpass.h"
#include "render/resource/render_ubo.h"


namespace RenderSystem
{
    enum
    {
        _main_camera_renderpass = 0,
        _ui_overlay_renderpass,
        _renderpass_count
    };

    class MainCameraRender : public RenderBase
    {
    public:
        ~MainCameraRender() override;

        void initialize() override;

        void setupRenderTargets();

        void setupBackupBuffer();

        void setViewport();

        void Tick() override;

        void draw();

        void loadSceneMeshes(std::vector<RenderMeshPtr> &visible_meshes);

        void setCameraMatrix(Matrix4x4 view, Matrix4x4 proj)
        {
            m_render_per_frame_ubo.per_frame_ubo_list.view_proj = proj * view;
            m_render_per_frame_ubo.ToGPU();
        }

        void setUIOverlay(UIOverlayPtr ui_overlay)
        {
            m_render_resource_info.p_ui_overlay = ui_overlay;
        }

    private:
        void setupCommandBuffer();

        void setupDescriptorPool();

        void updateAfterSwapchainRecreate();

    private:
        VkCommandPool                m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;
        VkDescriptorPool             m_descriptor_pool;
//        MainCameraRenderPass         renderPass;
        std::vector<RenderPassPtr>   m_render_passes;
        std::vector<ImageAttachment> m_render_targets;
        std::vector<ImageAttachment> m_backup_targets;
        std::vector<RenderMeshPtr>   m_visible_meshes;

        RenderModelUBOList m_render_model_ubo_list;
        RenderPerFrameUBO  m_render_per_frame_ubo;

        void setupRenderpass();
    };
}
#endif //XEXAMPLE_MAIN_CAMERA_RENDER_H
