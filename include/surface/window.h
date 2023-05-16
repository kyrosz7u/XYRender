//
// Created by kyrosz7u on 4/3/23.
//

#ifndef VULKANRENDER_SURFACE_H
#define VULKANRENDER_SURFACE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct WindowCreateInfo
{
    int         width {1280};
    int         height {720};
    const char* title {"DefaultTitle"};
    bool        is_fullscreen {false};
};

class window {
public:
    void initialize(WindowCreateInfo create_info);
private:
    GLFWwindow* m_window {nullptr};
    int         m_width {0};
    int         m_height {0};

    bool m_is_focus_mode {false};
};


#endif //VULKANRENDER_SURFACE_H
