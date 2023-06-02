//
// Created by kyrosz7u on 4/3/23.
//

#ifndef VULKANRENDER_SURFACE_H
#define VULKANRENDER_SURFACE_H

#define GLFW_INCLUDE_VULKAN

#include "logger/logger_macros.h"

#include <GLFW/glfw3.h>

struct GLFWWindowCreateInfo
{
    int width{1280};
    int height{720};
    const char *title{"DefaultTitle"};
    bool is_fullscreen{false};
};

class GLFWWindow
{
public:
    ~GLFWWindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void initialize(GLFWWindowCreateInfo create_info);

    void pollEvents();

    bool shouldClose();

    ::GLFWwindow *getWindowHandler()
    { return m_window; }

    bool isMouseButtonDown(int button) const;

    typedef std::function<void(int, int, int, int)> onKeyPressFunc;
    typedef std::function<void(unsigned int)>       onKeyCharFunc;
    typedef std::function<void(double, double)>     onCursorPosFunc;
    typedef std::function<void(int)>                onCursorEnterFunc;
    typedef std::function<void(int, int, int)>      onMouseButtonFunc;
    typedef std::function<void(double, double)>     onWindowContentScaleFunc;
    typedef std::function<void(int, int)>           onWindowSizeFunc;
    typedef std::function<void()>                   onWindowCloseFunc;

    // 窗口事件处理采用观察者模式，通过注册回调函数订阅事件
    void registerOnKeyFunc(onKeyPressFunc func)
    { m_keypress_events.push_back(func); }

    void registerOnCharFunc(onKeyCharFunc func)
    { m_keychar_events.push_back(func); }

    void registerOnCursorPosFunc(onCursorPosFunc func)
    { m_cursor_pos_events.push_back(func); }

    void registerOnCursorEnterFunc(onCursorEnterFunc func)
    { m_cursor_enter_events.push_back(func); }

    void registerOnWindowSizeFunc(onWindowSizeFunc func)
    { m_window_size_events.push_back(func); }

    void registerOnWindowCloseFunc(onWindowCloseFunc func)
    { m_window_close_events.push_back(func); }

    // TODO: 需要对订阅者对象的生命周期进行管理
    void onKeyPress(int key, int scancode, int action, int mods)
    {
        for (auto &func: m_keypress_events)
            func(key, scancode, action, mods);
    }

    void onKeyChar(unsigned int codepoint)
    {
        for (auto &func: m_keychar_events)
            func(codepoint);
    }

    void onCursorPos(double xpos, double ypos)
    {
        for (auto &func: m_cursor_pos_events)
            func(xpos, ypos);
    }

    void onCursorEnter(int entered)
    {
        for (auto &func: m_cursor_enter_events)
            func(entered);
    }

    void onMouseButton(int button, int action, int mods)
    {
        for (auto &func: m_mouse_button_events)
            func(button, action, mods);
    }

    void onWindowContentScale(float xscale, float yscale)
    {
        for (auto &func: m_window_content_scale_events)
            func(xscale, yscale);
    }

    void onWindowSize(int width, int height)
    {
        for (auto &func: m_window_size_events)
            func(width, height);
    }

    void onWindowClose()
    {
        for (auto &func: m_window_close_events)
            func();
    }

private:
    ::GLFWwindow *m_window{nullptr};
    int m_width{0};
    int m_height{0};

    bool m_is_focus_mode{false};

    std::vector<onKeyPressFunc>           m_keypress_events;
    std::vector<onKeyCharFunc>            m_keychar_events;
    std::vector<onCursorPosFunc>          m_cursor_pos_events;
    std::vector<onCursorEnterFunc>        m_cursor_enter_events;
    std::vector<onMouseButtonFunc>        m_mouse_button_events;
    std::vector<onWindowContentScaleFunc> m_window_content_scale_events;
    std::vector<onWindowSizeFunc>         m_window_size_events;
    std::vector<onWindowCloseFunc>        m_window_close_events;

    static void keyCallback(::GLFWwindow *window, int key, int scancode, int action, int mods);

    static void charCallback(::GLFWwindow *window, unsigned int codepoint);

    static void cursorPosCallback(::GLFWwindow *window, double xpos, double ypos);

    static void cursorEnterCallback(::GLFWwindow *window, int entered);

    static void mouseButtonCallback(::GLFWwindow *window, int button, int action, int mods);

    static void windowContentScaleCallback(GLFWwindow *window, float xscale, float yscale);

    static void windowSizeCallback(::GLFWwindow *window, int width, int height);

    static void windowCloseCallback(::GLFWwindow *window);
};

#endif //VULKANRENDER_SURFACE_H
