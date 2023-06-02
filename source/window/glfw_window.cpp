//
// Created by kyrosz7u on 4/3/23.
//

#include "window/glfw_window.h"

void GLFWWindow::initialize(GLFWWindowCreateInfo create_info)
{
    if (!glfwInit())
    {
        LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
        return;
    }

    m_width  = create_info.width;
    m_height = create_info.height;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(create_info.width, create_info.height, "123", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return;
    }

    // Setup input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
//    glfwSetCharModsCallback(m_window, charModsCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
//    glfwSetScrollCallback(m_window, scrollCallback);
//    glfwSetDropCallback(m_window, dropCallback);
    glfwSetWindowContentScaleCallback(m_window, windowContentScaleCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);
    glfwSetWindowCloseCallback(m_window, windowCloseCallback);

//    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

bool GLFWWindow::isMouseButtonDown(int button) const
{
    if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        return false;
    }
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}


void GLFWWindow::pollEvents()
{
    glfwPollEvents();
}

bool GLFWWindow::shouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void GLFWWindow::keyCallback(::GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLFWWindow* app = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if (app)
    {
        app->onKeyPress(key, scancode, action, mods);
    }
}

void GLFWWindow::charCallback(::GLFWwindow* window, unsigned int codepoint)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onKeyChar(codepoint);
}

void GLFWWindow::cursorPosCallback(::GLFWwindow* window, double xpos, double ypos)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onCursorPos(xpos, ypos);
}

void GLFWWindow::cursorEnterCallback(::GLFWwindow* window, int entered)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onCursorEnter(entered);
}

void GLFWWindow::mouseButtonCallback(::GLFWwindow* window, int button, int action, int mods)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onMouseButton(button, action, mods);
}

void GLFWWindow::windowContentScaleCallback(::GLFWwindow* window, float xscale, float yscale)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onWindowContentScale(xscale, yscale);
}

void GLFWWindow::windowSizeCallback(::GLFWwindow* window, int width, int height)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onWindowSize(width, height);
}

void GLFWWindow::windowCloseCallback(::GLFWwindow* window)
{
    auto* glfw_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if(glfw_window)
        glfw_window->onWindowClose();
}



