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
        SceneManager();

        ~SceneManager();

        void PostInitialize();

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
    private:
        friend class Camera;

        std::shared_ptr<RenderBase>        m_render;
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

