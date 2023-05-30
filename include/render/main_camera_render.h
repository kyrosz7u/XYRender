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
    class MainCameraRender : public RenderBase
    {
    public:
        ~MainCameraRender() override;

        void initialize() override;

        void setupRenderTargets();

        void setViewport();

        void Tick() override;

        void draw();

        void loadSceneMeshes(std::vector<RenderMeshPtr> &visible_meshes);

        void setCameraMatrix()
        {
            m_render_per_frame_ubo.per_frame_ubo_list.view_proj = Matrix4x4::IDENTITY;
            m_render_per_frame_ubo.ToGPU();
        }

    private:
        void setupCommandBuffer();

        void setupDescriptorPool();

        void updateAfterSwapchainRecreate();

    private:
        VkCommandPool                m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;
        VkDescriptorPool             m_descriptor_pool;
        MainCameraRenderPass         renderPass;
        std::vector<ImageAttachment> m_render_targets;
        std::vector<RenderMeshPtr>   m_visible_meshes;

        RenderModelUBOList m_render_model_ubo_list;
        RenderPerFrameUBO  m_render_per_frame_ubo;

        void setupRenderpass();
    };
}
#endif //XEXAMPLE_MAIN_CAMERA_RENDER_H
