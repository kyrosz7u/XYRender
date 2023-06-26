#include "scene/model.h"
#include "scene/camera.h"
#include "scene/direction_light.h"
#include "render/forward_render.h"
#include "render/defer_render.h"
#include "camera.h"
#include <memory>

#pragma once

namespace Scene
{
    class SceneManager : public std::enable_shared_from_this<SceneManager>
    {
    public:
        SceneManager()
        {
            auto window      = InputSystem.GetRawWindow();
            auto window_size = window->getWindowSize();
            m_main_camera = std::make_shared<Camera>(window_size.x / window_size.y,
                                                     90, 0.1f, 200.0f,
                                                     Scene::perspective);
            m_ui_overlay  = std::make_shared<UIOverlay>();
            m_ui_overlay->initialize(window);

            m_render = std::make_shared<DeferRender>();
            m_render->setUIOverlay(m_ui_overlay);
            m_render->initialize();
        }

        ~SceneManager()
        {
            // 必须显式调用render->destroy释放commandbuffer，
            // 否则会导致command还在执行的同时，destroy了资源
            // 因为成员变量的析构会晚于对象析构函数的调用
            m_render->destroy();
        }

        void AddModel(Model model)
        {
            m_models.push_back(model);
        }

        void AddLight(DirectionLight light)
        {
            m_directional_lights.push_back(light);
        }

        void LoadSkybox(const std::vector<std::string> &pathes)
        {
            m_skybox = std::make_shared<TextureCube>(pathes, "skybox");
        }

        void Tick();

        void PostInitialize();

    private:
        friend class Camera;

        std::shared_ptr<DeferRender>     m_render;
        UIOverlayPtr                       m_ui_overlay;
        // models
        std::vector<Scene::Model>          m_models;
        // mesh
        std::vector<RenderSubmesh>         m_visible_submeshes;
        // texture
        std::vector<Texture2DPtr>          m_visible_textures;
        std::shared_ptr<TextureCube>       m_skybox;
        // scence ubo
        std::shared_ptr<Camera>            m_main_camera;
        std::vector<Scene::DirectionLight> m_directional_lights;

        void updateScene();
    };
}

