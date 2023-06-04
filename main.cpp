#include "core/logger/logger_macros.h"
#include "core/window/glfw_window.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "render/resource/render_mesh.h"
#include "ui/ui_overlay.h"

int main() {
    GLFWWindowCreateInfo windowCreateInfo;
    GLFWWindow   window;
    UIOverlayPtr p_ui_overlay = std::make_shared<UIOverlay>();

    window.initialize(windowCreateInfo);
    p_ui_overlay->initialize(std::make_shared<GLFWWindow>(window));

    RenderBase::setupGlobally(window.getWindowHandler());

    Scene::Camera mainCamera;

    mainCamera.aspect= (float)windowCreateInfo.width / windowCreateInfo.height;
    mainCamera.fov   = 90;
    mainCamera.znear = 0.1f;
    mainCamera.zfar  = 100.0f;
    mainCamera.mode = Scene::perspective;
    mainCamera.position = Math::Vector3(0, 10, -20);
    mainCamera.rotation = Math::EulerAngle(0, 0, 0);

    window.registerOnWindowSizeFunc(mainCamera.windowSizeChangedDelegate);

    Scene::Model model;

    model.loadModelFile("assets/models/Kong.fbx");

    for (auto &mesh:model.m_meshes)
    {
        mesh->ToGPU();
    }

    mainCamera.render->loadSceneMeshes(model.m_meshes);
    mainCamera.render->setUIOverlay(p_ui_overlay);

    while(!window.shouldClose())
    {
        mainCamera.Tick();
        window.pollEvents();
    }
    return 0;
}
