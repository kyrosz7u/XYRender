#include <iostream>
#include "logger_macros.h"
#include "window/Window.h"
#include "scene/Camera.h"
#include "render/RenderBase.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main() {
    WindowCreateInfo windowCreateInfo;

    Window window;

    window.initialize(windowCreateInfo);

    RenderBases::setupGlobally(window.getWindowHandler());

    Camera mainCamera;

    while(!window.shouldClose())
    {
        mainCamera.Tick();
        window.pollEvents();
    }
    return 0;
}
