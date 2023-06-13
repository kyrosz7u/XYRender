
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

    for (int i = 0; i < m_models.size(); ++i)
    {
        auto            &model          = m_models[i];
        const auto      &model_textures = model.getTextures();
        for (const auto &texture: model_textures)
        {
            m_visible_textures.push_back(texture);
        }
    }
    m_render->UpdateRenderTextures(m_visible_textures);
    m_render->UpdateSkyboxTexture(m_skybox);
}

void SceneManager::updateScene()
{
    m_main_camera->Tick();

    m_visible_meshes.clear();
    m_visible_model_matrix.clear();

    int texture_offset = 0;

    for (int i = 0; i < m_models.size(); ++i)
    {
        auto       &model           = m_models[i];
        const auto &model_textures  = model.getTextures();
        const auto &model_submeshes = model.getSubmeshes();

        model.SetMeshIndex(i);

        for (auto       submesh: model_submeshes)
        {
            if (submesh.material_index >= 0)
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

    updateScene();
    m_render->UpdateRenderSubMesh(m_visible_meshes);
    m_render->UpdateRenderPerFrameScenceUBO(m_main_camera->getProjViewMatrix(),
                                            m_main_camera->position,
                                            m_directional_lights);
    m_render->Tick();
}


