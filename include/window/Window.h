//
// Created by kyrosz7u on 4/3/23.
//

#ifndef VULKANRENDER_SURFACE_H
#define VULKANRENDER_SURFACE_H

#define GLFW_INCLUDE_VULKAN
#include "macros.h"

#include <GLFW/glfw3.h>

struct WindowCreateInfo
{
    int         width {1280};
    int         height {720};
    const char* title {"DefaultTitle"};
    bool        is_fullscreen {false};
};

class Window {
public:
    ~Window()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void initialize(WindowCreateInfo create_info);
    void pollEvents();
    bool shouldClose();
    GLFWwindow* getWindowHandler(){return m_window;}

private:
    GLFWwindow* m_window {nullptr};
    int         m_width {0};
    int         m_height {0};

    bool m_is_focus_mode {false};
};


#endif //VULKANRENDER_SURFACE_H
