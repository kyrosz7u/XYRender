//
// Created by kyrosz7u on 2023/6/25.
//

#ifndef XEXAMPLE_DEFER_RENDER_H
#define XEXAMPLE_DEFER_RENDER_H

#include "render_base.h"
#include "renderpass/main_camera_forward_pass.h"
#include "render/resource/render_ubo.h"
#include "render/resource/render_resource.h"
#include "render/shadow.h"
#include "scene/model.h"
#include "scene/direction_light.h"
#include "core/threadpool.h"
#include "core/logger/logger_macros.h"

namespace RenderSystem
{

    class DeferRender : public RenderBase
    {
    public:
        enum _renderpasses
        {
            _directional_light_shadowmap_renderpass = 0,
            _main_camera_renderpass,
            _ui_overlay_renderpass,
            _renderpass_count
        };

        struct ThreadData
        {
            int                          thread_index;
            RenderPassPtr                p_renderpass;
            VkCommandPool                commandPool;
            std::vector<VkCommandBuffer> secondary_command_buffers;
        };

        DeferRender()
                : m_render_light_project_ubo_list(MAX_DIRECTIONAL_LIGHT_COUNT * sizeof(VulkanLightProjectDefine))
        {}

        void initialize() override;

        void postInitialize() override;

        void destroy() override;

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

        void UpdateRenderResource(const std::vector<Scene::Model> &_visible_models,
                                  const std::vector<RenderSubmesh> &_visible_submeshes,
                                  const std::vector<Scene::DirectionLight> &directional_light_list,
                                  const Scene::Camera &main_camera) override;

        void SetupModelRenderTextures(const std::vector<Texture2DPtr> &_visible_textures) override;

        void SetupSkyboxTexture(const std::shared_ptr<TextureCube> &skybox_texture) override;

        void SetupShadowMapTexture(std::vector<Scene::DirectionLight> &directional_light_list) override;

        void FlushRenderbuffer() override;

    private:
        void setupCommandBuffer();

        void setupDescriptorPool();

        void setupRenderpass();

        void UpdateRenderModelList(const std::vector<Scene::Model> &_visible_models,
                                   const std::vector<RenderSubmesh> &_visible_submeshes) override;

        void UpdateRenderPerFrameScenceUBO(const Matrix4x4 proj_view,
                                           const Vector3 camera_pos,
                                           const std::vector<Scene::DirectionLight> &directional_light_list) override;

        void UpdateLightAndShadowDataList(const std::vector<Scene::DirectionLight> &directional_light_list,
                                          const Scene::Camera &main_camera) override;

        void updateAfterSwapchainRecreate();

        void setupRenderDescriptorSetLayout();

        // render resource
    private:
        Matrix4x4 m_view_matrix;
        Matrix4x4 m_proj_matrix;

        std::vector<DirLightShadow>              m_directional_light_shadow_list;
        const std::vector<Scene::DirectionLight> *m_directional_light_list;
        const std::vector<Scene::Model>          *m_visible_models;
        const std::vector<RenderSubmesh>         *m_visible_submeshes;
        const Camera                             *m_main_camera;

        // api resource
    private:
        VkCommandPool                m_primary_command_pool{VK_NULL_HANDLE};
        std::vector<VkCommandBuffer> m_primary_command_buffers;
        // multi thread
        ThreadPool                   m_thread_pool;
        std::vector<ThreadData>      m_thread_datas;

        VkDescriptorPool                 m_descriptor_pool{VK_NULL_HANDLE};
        std::vector<RenderPassPtr>       m_render_passes;
        // render target
        std::vector<ImageAttachment>     m_render_targets;
        std::vector<ImageAttachment>     m_backup_targets;
        ImageAttachment                  m_directional_light_shadowmap;
        // render submesh cache
        std::vector<RenderSubmesh>       m_render_submeshes;
        // ubo
        RenderPerFrameUBO                m_render_per_frame_ubo;
        RenderModelUBOList               m_render_model_ubo_list;
        RenderLightProjectUBOList        m_render_light_project_ubo_list;
        RenderShadowMapSampleDataUBOList m_render_shadow_map_sample_data_ubo_list;
        // texture info list
        VkDescriptorSetLayout            m_texture_descriptor_set_layout{VK_NULL_HANDLE};
        std::vector<VkDescriptorSet>     m_texture_descriptor_sets;
        // directional light info list
        VkDescriptorSetLayout            m_directional_light_shadow_set_layout{VK_NULL_HANDLE};
        VkDescriptorSet                  m_directional_light_shadow_set{VK_NULL_HANDLE};    // use texture array
        // skybox info
        VkDescriptorSetLayout            m_skybox_descriptor_set_layout{VK_NULL_HANDLE};
        VkDescriptorSet                  m_skybox_descriptor_set{VK_NULL_HANDLE};

        void updateLightShadow(std::vector<Scene::DirectionLight> &directional_light_list,
                               const std::shared_ptr<Scene::Camera> &main_camera);
    };
}
#endif //XEXAMPLE_DEFER_RENDER_H
