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
    void addDebugDrawCommand(std::function<void()> command)
    {
        m_debug_draw_commands.push_back(command);
    }

private:
    bool m_initialized = false;
    std::shared_ptr<GLFWWindow> m_window;
    std::vector<std::function<void()>> m_debug_draw_commands;
};

