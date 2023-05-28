#include <iostream>
#include "logger_macros.h"
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

    Scene::Model model;

    model.loadModelFile("assets/models/Kong.fbx");

    for (auto &mesh:model.m_meshes)
    {
        mesh->ToDevice();
    }

    while(!window.shouldClose())
    {
        mainCamera.Tick();
        window.pollEvents();
    }
    return 0;
}
