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


    LOG_INFO("12345")
    LOG_DEBUG("67890")
    LOG_ERROR("abcde%s","aaa")
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
