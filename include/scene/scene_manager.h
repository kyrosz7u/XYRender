#include "scene/model.h"
#include "scene/camera.h"
#include "scene/direction_light.h"
#include <memory>

#pragma once

namespace Scene
{
    class SceneManager : std::enable_shared_from_this<SceneManager>
    {
    public:
        SceneManager()
        {}

        void addModel(Model model)
        {
            m_models.push_back(model);
        }

        void addLight(DirectionLight light)
        {
            m_directional_lights.push_back(light);
        }

        void setUIOverlay(UIOverlayPtr overlay)
        {
            m_ui_overlay = overlay;
        }

    private:
        std::vector<Scene::Model>          m_models;
        Scene::Camera                      m_main_camera;
        std::vector<Scene::DirectionLight> m_directional_lights;
        UIOverlayPtr                       m_ui_overlay;
    };
}

