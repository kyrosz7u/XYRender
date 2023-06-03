#include "window/glfw_window.h"
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
    void initialize(std::shared_ptr<GLFWWindow> window)
    {
        m_window = window;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // set ui content scale
        float x_scale, y_scale;
        glfwGetWindowContentScale(m_window->getWindowHandler(), &x_scale, &y_scale);
        float content_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
//        glfwSetWindowContentScaleCallback(init_info.window_system->getWindow(), windowContentScaleCallback);

        // load font for imgui
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//        io.ConfigWindowsMoveFromTitleBarOnly = true;
//        io.Fonts->AddFontFromFileTTF(
//                "assets/fonts/PingFang Regular.ttf", content_scale * 16, nullptr, nullptr);
//        io.Fonts->Build();

        ImGui::StyleColorsDark();

//        ImGuiStyle& style     = ImGui::GetStyle();
//        style.WindowPadding   = ImVec2(1.0, 0);
//        style.FramePadding    = ImVec2(14.0, 2.0f);
//        style.ChildBorderSize = 0.0f;
//        style.FrameRounding   = 5.0f;
//        style.FrameBorderSize = 1.5f;

        m_initialized = true;
    }

    // imgui perframe draw command
    void drawImGui()
    {
        float light_speed = 0.5f;
        static bool show_another_window = true;

        ImGui::Begin("Hello, world!");
//        ImGui::ShowDemoWindow();
        ImGui::End();

        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();

    }

private:
    bool m_initialized = false;
    std::shared_ptr<GLFWWindow> m_window;
};

