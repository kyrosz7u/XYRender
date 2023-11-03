#include "scene/scene_manager.h"
#include "core/logger/logger_macros.h"

using namespace Scene;

SceneManager::SceneManager()
{
    auto window      = InputSystem.GetRawWindow();
    auto window_size = window->getWindowSize();
    m_main_camera = std::make_shared<Camera>(window_size.x / window_size.y,
                                             90, 0.1f, 200.0f,
                                             Scene::perspective);
    m_ui_overlay  = std::make_shared<UIOverlay>();
    m_ui_overlay->initialize(window);
#ifdef DEFERRED_RENDERING
    m_render = std::make_shared<DeferRender>();
#elif FORWARD_RENDERING
    m_render = std::make_shared<ForwardRender>();
#else
    LOG_ERROR("No render type specified! Please define DEFER_RENDERING or FORWARD_RENDERING in CMakeLists.txt.")
    throw std::runtime_error(
            "No render type specified! Please define DEFER_RENDERING or FORWARD_RENDERING in CMakeLists.txt.");
#endif
    m_render->setUIOverlay(m_ui_overlay);
    m_render->initialize();
}

SceneManager::~SceneManager()
{
    // 必须显式调用render->destroy释放commandbuffer，
    // 否则会导致command还在执行的同时，destroy了资源
    // 因为成员变量的析构会晚于对象析构函数的调用
    m_render->destroy();
}

void SceneManager::PostInitialize()
{
    m_main_camera->setParentScene(shared_from_this());
    m_main_camera->position = Math::Vector3(0, 10, -20);
    m_main_camera->rotation = Math::EulerAngle(0, 0, 0);
    m_main_camera->fov      = 90.0f;
    m_main_camera->PostInitialize();

    //m_ui_overlay->addDebugDrawCommand(std::bind(&_InputSystem::ImGuiDebugPanel, &_InputSystem::Instance()));
    //m_ui_overlay->addDebugDrawCommand(std::bind(&Scene::Camera::ImGuiDebugPanel, m_main_camera));
    m_ui_overlay->addDebugDrawCommand(std::bind(&Scene::DirectionLight::ImGuiDebugPanel, &m_directional_lights[0]));

    for (int i = 0; i < m_models.size(); ++i)
    {
        auto            &model          = m_models[i];
        const auto      &model_textures = model.GetTextures();
        for (const auto &texture: model_textures)
        {
            m_visible_textures.push_back(texture);
        }
    }

    m_render->SetupModelRenderTextures(m_visible_textures);
    m_render->SetupSkyboxTexture(m_skybox);
    m_render->SetupShadowMapTexture(m_directional_lights);

    m_render->postInitialize();
}

void SceneManager::updateScene()
{
    m_main_camera->Tick();

    m_visible_submeshes.clear();

    int texture_offset = 0;

    for (int i = 0; i < m_models.size(); ++i)
    {
        auto       &model           = m_models[i];
        const auto &model_textures  = model.GetTextures();
        const auto &model_submeshes = model.GetSubmeshes();

        model.Tick();
        model.SetMeshIndex(i);

        for (auto submesh: model_submeshes)
        {
            if (submesh.material_index >= 0)
                submesh.material_index += texture_offset;
            m_visible_submeshes.push_back(submesh);
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
    //m_render->UpdateRenderModelList(m_models, m_visible_submeshes);
    //m_render->UpdateRenderPerFrameScenceUBO(m_main_camera->getProjViewMatrix(),
    //                                        m_main_camera->position,
    //                                        m_directional_lights);
    //m_render->UpdateLightAndShadowDataList(m_directional_lights, m_main_camera);
    m_render->UpdateRenderResource(m_models,
                                   m_visible_submeshes,
                                   m_directional_lights,
                                   *m_main_camera);
    m_render->Tick();
}


