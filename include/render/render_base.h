#include <GLFW/glfw3.h>
#include "core/graphic/vulkan/vulkan_context.h"
#include "render/common_define.h"
#include "render/renderpass/renderpass_base.h"
#include "render/subpass/subpass_base.h"
#include "render/resource/render_resource.h"
#include "render/resource/render_mesh.h"
#include "render/resource/render_ubo.h"
#include <memory>

#pragma once
namespace Scene
{
    class DirectionLight;
    class Camera;
    class Model;
}

namespace RenderSystem
{
    using namespace VulkanAPI;
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    class RenderBase
    {
    public:
        VkViewport m_viewport = {0, 0, 1280, 768, 0, 1};
        VkRect2D   m_scissor  = {{0,    0},
                                 {1280, 768}};


    public:
        RenderBase()
        {
            assert(g_p_vulkan_context != nullptr);
        }

        virtual ~RenderBase() = default;

        virtual void initialize() = 0;

        virtual void postInitialize() = 0;

        virtual void destroy() = 0;

        static void setupGlobally(GLFWwindow *window);

        void setUIOverlay(UIOverlayPtr ui_overlay)
        {
            m_p_ui_overlay = ui_overlay;
        }

        virtual void UpdateRenderResource(const std::vector<Scene::Model> &_visible_models,
                                          const std::vector<RenderSubmesh> &_visible_submeshes,
                                          const std::vector<Scene::DirectionLight> &directional_light_list,
                                          const Scene::Camera &main_camera)
        {}


        virtual void UpdateRenderModelList(const std::vector<Scene::Model> &_visible_models,
                                           const std::vector<RenderSubmesh> &_visible_submeshes)
        {}

        virtual void UpdateRenderPerFrameScenceUBO(const Matrix4x4 proj_view,
                                                   const Vector3 camera_pos,
                                                   const std::vector<Scene::DirectionLight> &directional_light_list)
        {}

        virtual void SetupModelRenderTextures(const std::vector<Texture2DPtr> &_visible_textures)
        {}

        virtual void SetupSkyboxTexture(const std::shared_ptr<TextureCube> &skybox_texture)
        {}

        virtual void UpdateLightProjectionList(const std::vector<Scene::DirectionLight> &directional_light_list,
                                               const Scene::Camera &main_camera)
        {}

        virtual void SetupShadowMapTexture(std::vector<Scene::DirectionLight> &directional_light_list)
        {}

        virtual void FlushRenderbuffer()
        {}

        virtual void Tick()
        {
            if (m_frame_count == 0)
                m_last_frame_time = glfwGetTimerValue();

            m_frame_count++;
            m_current_frame_time = glfwGetTimerValue();
            m_frame_time         = (float) (m_current_frame_time - m_last_frame_time) / glfwGetTimerFrequency();
            m_last_frame_time    = m_current_frame_time;
        }

        inline uint64_t getFrameCount() const
        {
            return m_frame_count;
        }

        inline float getFrameTime() const
        {
            return m_frame_time;
        }

        inline Vector2 getShadowMapSize() const
        {
            return Vector2(m_render_resource_info.kDirectionalLightInfo.shadowmap_width,
                           m_render_resource_info.kDirectionalLightInfo.shadowmap_height);
        }

    protected:
        RenderGlobalResourceInfo     m_render_resource_info;
        VulkanAPI::RenderCommandInfo m_render_command_info;
        uint32_t                     m_frame_count{0};
        float                        m_frame_time{0};
        UIOverlayPtr                 m_p_ui_overlay;
    private:
        uint64_t m_last_frame_time{0};
        uint64_t m_current_frame_time{0};

        void setupCommandBuffer();
    };
}

