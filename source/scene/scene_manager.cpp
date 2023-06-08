
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

void SceneManager::updateScene()
{
    m_visible_meshes.clear();
    m_visible_textures.clear();
    m_visible_model_matrix.clear();

    m_per_frame_scene_define.camera_pos = m_main_camera->position;
    m_per_frame_scene_define.proj_view =
            m_main_camera->calculatePerspectiveMatrix() * m_main_camera->calculateViewMatrix();

    m_visible_model_matrix.resize(m_models.size());
    int texture_offset = 0;

    for (int i = 0; i < m_models.size(); ++i)
    {
        auto       &model           = m_models[i];
        const auto &model_textures  = model.getTextures();
        const auto &model_submeshes = model.getSubmeshes();

        model.SetMeshIndex(i);
        m_visible_model_matrix[i].model = model.transform.GetTransformMatrix();

        for (const auto &texture: model_textures)
        {
            m_visible_textures.push_back(texture);
        }
        for (auto       submesh: model_submeshes)
        {
            if(submesh.material_index >= 0)
                submesh.material_index += texture_offset;
            m_visible_meshes.push_back(submesh);
        }
        texture_offset += model_textures.size();
    }
}

void SceneManager::Tick()
{
    for (auto &model: m_models)
    {
        model.Tick();
    }

    m_main_camera->Tick();

    updateScene();
    m_render->SetVisibleRenderData(&m_visible_meshes, &m_visible_textures);
    m_render->UpdateRenderModelUBOList(m_visible_model_matrix);
    m_render->UpdateRenderPerFrameScenceUBO(m_per_frame_scene_define);

    m_render->Tick();
}


