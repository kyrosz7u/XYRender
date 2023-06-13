//
// Created by kyrosz7u on 2023/5/19.
//

#ifndef XEXAMPLE_FORWARD_RENDER_H
#define XEXAMPLE_FORWARD_RENDER_H

#include "render_base.h"
#include "renderpass/main_camera_renderpass.h"
#include "render/resource/render_ubo.h"
#include "scene/model.h"
#include "scene/direction_light.h"

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

        void initialize() override;

        void destroy();

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

        void UpdateRenderModel(const std::vector<Scene::Model> &_visible_models);

        void UpdateRenderPerFrameScenceUBO(Matrix4x4 proj_view, Vector3 camera_pos,
                                           std::vector<Scene::DirectionLight> &directional_light_list);

        void UpdateModelRenderTextures(const std::vector<Texture2DPtr> &_visible_textures);

        void UpdateSkyboxTexture(const std::shared_ptr<TextureCube>& skybox_texture);

    private:

        void setupCommandBuffer();

        void setupDescriptorPool();

        void setupRenderpass();

        void updateAfterSwapchainRecreate();

        void setupRenderDescriptorSetLayout();

    private:
        VkCommandPool                m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;
        VkDescriptorPool             m_descriptor_pool;
        std::vector<RenderPassPtr>   m_render_passes;
        // render target
        std::vector<ImageAttachment> m_render_targets;
        std::vector<ImageAttachment> m_backup_targets;
        // render submesh cache
        std::vector<RenderSubmesh>   m_render_submeshes;
        // ubo
        RenderModelUBOList           m_render_model_ubo_list;
        RenderPerFrameUBO            m_render_per_frame_ubo;
        // texture info list
        VkDescriptorSetLayout        m_texture_descriptor_set_layout;
        std::vector<VkDescriptorSet> m_texture_descriptor_sets;

        // skybox info
        VkDescriptorSetLayout        m_skybox_descriptor_set_layout{VK_NULL_HANDLE};
        VkDescriptorSet              m_skybox_descriptor_set{VK_NULL_HANDLE};

        Matrix4x4 m_view_matrix;
        Matrix4x4 m_proj_matrix;

        UIOverlayPtr m_p_ui_overlay;
    };
}
#endif //XEXAMPLE_FORWARD_RENDER_H
