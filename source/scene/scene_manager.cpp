
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

void SceneManager::Tick()
{
    std::vector<RenderMeshPtr> visible_meshes;
    m_main_camera->Tick();
    for (auto &model : m_models)
    {
        model.Tick();
        visible_meshes.push_back(model.loaded_mesh);
    }
    m_render->LoadVisibleMeshes(visible_meshes);
    m_render->Tick();
}


