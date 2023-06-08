
#include "scene/scene_manager.h"

using namespace Scene;

void SceneManager::PostInitialize()
{
    m_main_camera->setParentScene(shared_from_this());
    m_main_camera->position = Math::Vector3(0, 10, -20);
    m_main_camera->rotation = Math::EulerAngle(0, 0, 0);
    m_main_camera->PostInitialize();

    m_ui_overlay->addDebugDrawCommand(std::bind(&_InputSystem::ImGuiDebugPanel, &_InputSystem::Instance()));
    m_ui_overlay->addDebugDrawCommand(std::bind(&Scene::Camera::ImGuiDebugPanel, m_main_camera));
}

void SceneManager::getVisibleMeshesAndTexture(std::vector<RenderSubmesh> &visible_meshes,
                                              std::vector<Texture2DPtr> &textures)
{
    int       submesh_count  = 0;
    int       texture_offset = 0;
    for (auto &model: m_models)
    {
        auto model_textures = model.getTextures();
        auto model_submeshes = model.getSubmeshes();
        for (auto &texture: model_textures)
        {
            textures.push_back(texture);
        }
        for (auto submesh: model_submeshes)
        {
            submesh.material_index+=texture_offset;
            visible_meshes.push_back(submesh);
        }
        texture_offset += model_textures.size();
    }
}

void SceneManager::Tick()
{
    std::vector<RenderMeshPtr> visible_meshes;
    m_main_camera->Tick();
    for (auto &model: m_models)
    {
        model.Tick();
        visible_meshes.push_back(model.mesh_loaded);
    }
    m_render->LoadVisibleMeshes(visible_meshes);


    m_render->Tick();
}


