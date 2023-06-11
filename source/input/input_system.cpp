//
// Created by kyros on 2023/6/4.
//
#include "input/input_system.h"
#include <imgui.h>
#include <map>

static const std::map<_InputSystem::CameraCommand, std::string> camera_command_name_map = {
        {_InputSystem::CameraCommand::no_command,    "no_command"},
        {_InputSystem::CameraCommand::camera_left,   "camera_left"},
        {_InputSystem::CameraCommand::camera_back,   "camera_back"},
        {_InputSystem::CameraCommand::camera_foward, "camera_foward"},
        {_InputSystem::CameraCommand::camera_right,  "camera_right"},
        {_InputSystem::CameraCommand::camera_up,     "camera_up"},
        {_InputSystem::CameraCommand::camera_down,   "camera_down"},
};

void _InputSystem::onKey(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_ESCAPE:
                m_is_focus_mode = false;
                m_window->setCursorFocus(m_is_focus_mode);
                break;
            case GLFW_KEY_LEFT_ALT:
                m_is_focus_mode = true;
                m_window->setCursorFocus(m_is_focus_mode);
                break;
            case GLFW_KEY_LEFT_SHIFT:
                SpeedUp = true;
                break;
            case GLFW_KEY_R:
                break;
            case GLFW_KEY_A:
                m_camera_command |= (unsigned int) CameraCommand::camera_left;
                break;
            case GLFW_KEY_S:
                m_camera_command |= (unsigned int) CameraCommand::camera_back;
                break;
            case GLFW_KEY_W:
                m_camera_command |= (unsigned int) CameraCommand::camera_foward;
                break;
            case GLFW_KEY_D:
                m_camera_command |= (unsigned int) CameraCommand::camera_right;
                break;
            case GLFW_KEY_Q:
                m_camera_command |= (unsigned int) CameraCommand::camera_up;
                break;
            case GLFW_KEY_E:
                m_camera_command |= (unsigned int) CameraCommand::camera_down;
                break;
        }
    } else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_LEFT_SHIFT:
                SpeedUp = false;
                break;
            case GLFW_KEY_A:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_left;
                break;
            case GLFW_KEY_S:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_back;
                break;
            case GLFW_KEY_W:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_foward;
                break;
            case GLFW_KEY_D:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_right;
                break;
            case GLFW_KEY_Q:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_up;
                break;
            case GLFW_KEY_E:
                m_camera_command &= ~(unsigned int) CameraCommand::camera_down;
                break;
        }
    }
}

void _InputSystem::onCursorPos(double current_cursor_x, double current_cursor_y)
{
    if (m_is_focus_mode)
    {
        m_cursor_delta_x = m_last_cursor_x - current_cursor_x;
        m_cursor_delta_y = m_last_cursor_y - current_cursor_y;
    }

    m_last_cursor_x = current_cursor_x;
    m_last_cursor_y = current_cursor_y;
}

void _InputSystem::initialize(std::shared_ptr<GLFWWindow> window)
{
    m_window = window;

    window->registerOnKeyFunc(std::bind(&_InputSystem::onKey, this, std::placeholders::_1, std::placeholders::_2,
                                        std::placeholders::_3, std::placeholders::_4));
    window->registerOnCursorPosFunc(std::bind(&_InputSystem::onCursorPos, this, std::placeholders::_1,
                                              std::placeholders::_2));
}

void _InputSystem::Tick()
{
    m_cursor_delta_x=0;
    m_cursor_delta_y=0;

    m_window->pollEvents();
    Look    = Math::Vector2(m_cursor_delta_x, m_cursor_delta_y);
    Move    = Math::Vector3::ZERO;
    Focused = m_is_focus_mode;

    if (m_camera_command & (unsigned int) CameraCommand::camera_left)
    {
        Move.x -= 1.0f;
    }
    if (m_camera_command & (unsigned int) CameraCommand::camera_back)
    {
        Move.z -= 1.0f;
    }
    if (m_camera_command & (unsigned int) CameraCommand::camera_foward)
    {
        Move.z += 1.0f;
    }
    if (m_camera_command & (unsigned int) CameraCommand::camera_right)
    {
        Move.x += 1.0f;
    }
    if (m_camera_command & (unsigned int) CameraCommand::camera_up)
    {
        Move.y += 1.0f;
    }
    if (m_camera_command & (unsigned int) CameraCommand::camera_down)
    {
        Move.y -= 1.0f;
    }
}

void _InputSystem::ImGuiDebugPanel()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("InputSystem"))
    {
        ImGui::Text("Focused: %s", Focused ? "true" : "false");
        ImGui::Text("cursor_delta: (%f, %f)", m_cursor_delta_x, m_cursor_delta_y);
        ImGui::Text("Move: (%f, %f, %f)", Move.x, Move.y, Move.z);
        ImGui::Text("Look: (%f, %f)", Look.x, Look.y);
        ImGui::TreePop();
    }
}

