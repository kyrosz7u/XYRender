#include <iostream>
#include "logger/logger_macros.h"
#include "window/Window.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "render/render_base.h"
#include "render/resource/render_mesh.h"

int main() {
    WindowCreateInfo windowCreateInfo;

    Window window;

    window.initialize(windowCreateInfo);

    RenderBase::setupGlobally(window.getWindowHandler());

    Scene::Camera mainCamera;

    mainCamera.aspect= windowCreateInfo.width / windowCreateInfo.height;
    mainCamera.fov = 90;
    mainCamera.zNear = 0.1f;
    mainCamera.zFar = 1000.0f;
    mainCamera.mode = Scene::perspective;
    mainCamera.position = Math::Vector3(0, 0, -10);
    mainCamera.rotation = Math::EulerAngle(0, 0, 0);

    Scene::Model model;

    model.loadModelFile("assets/models/Kong.fbx");

    for (auto &mesh:model.m_meshes)
    {
        mesh->ToGPU();
    }

    mainCamera.render->loadSceneMeshes(model.m_meshes);

    while(!window.shouldClose())
    {
        mainCamera.Tick();
        window.pollEvents();
    }
    return 0;
}
