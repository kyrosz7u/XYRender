#include "core/logger/logger_macros.h"
#include "core/window/glfw_window.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "scene/scene_manager.h"
#include "render/resource/render_mesh.h"
#include "input/input_system.h"
#include "ui/ui_overlay.h"

int main()
{
    GLFWWindowCreateInfo windowCreateInfo;
    auto           window = std::make_shared<GLFWWindow>();

    window->initialize(windowCreateInfo);

    InputSystem.initialize(window);
    RenderBase::setupGlobally(window->getWindowHandler());

    Scene::Model model;

    auto scene_manager = std::make_shared<Scene::SceneManager>();

    model.LoadModelFile("assets/models/Kong.fbx","Kong");
    model.ToGPU();
    scene_manager->AddModel(model);

//    mainCamera.m_render->loadSingleMesh(model.mesh_loaded);
    model.LoadModelFile("assets/models/capsule.obj", "capsule");
    model.transform.position = Math::Vector3(10, 10, 0);
//    model.transform.rotation = Math::Vector3(90, 0, 0);
    model.ToGPU();
    scene_manager->AddModel(model);

//    model.LoadModelFile("assets/models/floor.obj", "floor");
//    model.transform.position = Math::Vector3(0, 0, 0);
//    model.ToGPU();
//    scene_manager->AddModel(model);

    Scene::DirectionLight light;
    light.intensity = 1.0f;
    light.transform = Transform(Math::Vector3(0, 15, -15), Math::EulerAngle(50, -30, 0), Math::Vector3(1, 1, 1));
    light.color     = Color(1, float(244 / 255), float(214 / 255), 255);
    scene_manager->AddLight(light);

    scene_manager->PostInitialize();

    while (!window->shouldClose())
    {
        InputSystem.Tick();
        scene_manager->Tick();
    }
    return 0;
}
