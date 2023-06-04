#include "core/window/glfw_window.h"
#include <imgui.h>
#include <memory>

#pragma once

class UIOverlay;
typedef std::shared_ptr<UIOverlay> UIOverlayPtr;

class UIOverlay
{
public:
    UIOverlay()
    {

    }
    bool isInitialized()
    {
        return m_initialized;
    }

    void initialize(std::shared_ptr<GLFWWindow> window);
    void drawImGui();

    // imgui perframe draw command

private:
    bool m_initialized = false;
    std::shared_ptr<GLFWWindow> m_window;
};

