//
// Created by kyros on 2023/6/4.
//

#include "ui/ui_overlay.h"

static inline void windowContentScaleUpdate(float scale)
{
#if defined(__GNUC__) && defined(__MACH__)
    float font_scale               = fmaxf(1.0f, scale);
        ImGui::GetIO().FontGlobalScale = 1.0f / font_scale;
#endif
    // TOOD: Reload fonts if DPI scale is larger than previous font loading DPI scale
}

static inline void windowContentScaleCallback(GLFWwindow* window, float x_scale, float y_scale)
{
    windowContentScaleUpdate(fmaxf(x_scale, y_scale));
}

void UIOverlay::initialize(std::shared_ptr<GLFWWindow> window)
{
    m_window = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // set ui content scale
    float x_scale, y_scale;
    glfwGetWindowContentScale(m_window->getWindowHandler(), &x_scale, &y_scale);
    float content_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
    glfwSetWindowContentScaleCallback(m_window->getWindowHandler(), windowContentScaleCallback);

    // load font for imgui
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.Fonts->AddFontFromFileTTF(
            "assets/fonts/PingFang Regular.ttf", content_scale * 15, nullptr, nullptr);
    io.Fonts->Build();

    ImGui::StyleColorsDark();

//        ImGuiStyle& style     = ImGui::GetStyle();
//        style.WindowPadding   = ImVec2(1.0, 0);
//        style.FramePadding    = ImVec2(14.0, 2.0f);
//        style.ChildBorderSize = 0.0f;
//        style.FrameRounding   = 5.0f;
//        style.FrameBorderSize = 1.5f;

    m_initialized = true;
}

void UIOverlay::drawImGui()
{
    float light_speed = 0.5f;
    static bool show_another_window = true;

    ImGui::Begin("Hello, world!");
    ImGui::ShowDemoWindow();
    ImGui::End();

    if(show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}


