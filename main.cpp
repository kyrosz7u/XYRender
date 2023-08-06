#include "core/logger/logger_macros.h"
#include "core/window/glfw_window.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "scene/scene_manager.h"
#include "render/resource/render_mesh.h"
#include "input/input_system.h"
#include "ui/ui_overlay.h"

std::shared_ptr<GLFWWindow> window = nullptr;
std::shared_ptr<Scene::SceneManager> scene_manager = nullptr;

void globalInit();

int main()
{
    globalInit();
    
    Scene::Model model;
    model.LoadModelFile("assets/models/Kong.fbx","Kong");
    model.ToGPU();
    scene_manager->AddModel(model);

//    mainCamera.m_render->loadSingleMesh(model.mesh_loaded);
    model.LoadModelFile("assets/models/capsule.obj", "capsule");
    model.transform.position = Math::Vector3(10, 10, 0);
    model.transform.scale= Math::Vector3(8.0f, 10.0f, 12.0f);
//    model.transform.rotation = Math::Vector3(90, 0, 0);
    model.ToGPU();
    scene_manager->AddModel(model);

    model.LoadModelFile("assets/models/plane.obj", "plane");
    model.transform.position = Math::Vector3(0, 0, 0);
    model.transform.scale    = Math::Vector3(6.0f, 1.0f, 6.0f);
    model.ToGPU();
    scene_manager->AddModel(model);

    // +X，-X，+Y，-Y，+Z，-Z
    std::vector<std::string> skybox_faces = {
            "assets/textures/skybox/right.jpg",
            "assets/textures/skybox/left.jpg",
            "assets/textures/skybox/top.jpg",
            "assets/textures/skybox/bottom.jpg",
            "assets/textures/skybox/front.jpg",
            "assets/textures/skybox/back.jpg"};
    scene_manager->LoadSkybox(skybox_faces);

    Scene::DirectionLight light;
    light.intensity = 1.0f;
    light.transform = Transform(Math::Vector3(0, 15, -15), Math::EulerAngle(60, -30, 0), Math::Vector3(1, 1, 1));
//    light.transform.rotation = Math::EulerAngle(90, 0, 0);
//    light.transform = Transform(Math::Vector3(0, 15, -15), Math::EulerAngle(0, 0, 0), Math::Vector3(1, 1, 1));
    light.color     = Color(1, float(244) / 255, float(214) / 255, 1.0f);
    scene_manager->AddLight(light);

    scene_manager->PostInitialize();
    while (!window->shouldClose())
    {
        InputSystem.Tick();
        scene_manager->Tick();
    }
    return 0;
}

void globalInit()
{
    GLFWWindowCreateInfo windowCreateInfo;
    window = std::make_shared<GLFWWindow>();

    window->initialize(windowCreateInfo);

    InputSystem.initialize(window);
    RenderBase::setupGlobally(window->getWindowHandler());

    scene_manager = std::make_shared<Scene::SceneManager>();
}

