#include "scene/model.h"
#include "scene/camera.h"
#include "scene/direction_light.h"
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
                                                     90, 0.1f, 100.0f,
                                                     Scene::perspective);
            m_ui_overlay = std::make_shared<UIOverlay>();
            m_ui_overlay->initialize(window);

            m_render = std::make_shared<ForwardRender>();
            m_render->setUIOverlay(m_ui_overlay);
            m_render->initialize();
        }

        void AddModel(Model model)
        {
            m_models.push_back(model);
        }

        void AddLight(DirectionLight light)
        {
            m_directional_lights.push_back(light);
        }

        void Tick();

        void PostInitialize();

    private:
        friend class Camera;

        std::vector<Scene::Model>          m_models;
        std::shared_ptr<Camera>            m_main_camera;
        std::vector<Scene::DirectionLight> m_directional_lights;
        UIOverlayPtr                       m_ui_overlay;

        std::shared_ptr<ForwardRender> m_render;
    };
}

