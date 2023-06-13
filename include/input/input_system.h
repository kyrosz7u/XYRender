//
// Created by kyros on 2023/6/4.
//

#ifndef XEXAMPLE_INPUT_SYSTEM_H
#define XEXAMPLE_INPUT_SYSTEM_H

#include "core/singleton_decorator.h"
#include "core/window/glfw_window.h"
#include "core/math/math.h"

#define InputSystem _InputSystem::Instance()

// main函数执行前，先执行SingletonDecorator<_InputSystem>::createInstance()，创建InputSystem实例
// 需要
class _InputSystem : public SingletonDecorator<_InputSystem>
{
public:
    enum CameraCommand : unsigned int
    {
        no_command    = 0,
        camera_left   = 1 << 0,  // A
        camera_back   = 1 << 1,  // S
        camera_foward = 1 << 2,  // W
        camera_right  = 1 << 3,  // D
        camera_up     = 1 << 4,  // Q
        camera_down   = 1 << 5,  // E
    };

    Math::Vector2 Look;
    Math::Vector3 Move;
    bool SpeedUp{false};
    bool Focused{false};

    void initialize(std::shared_ptr<GLFWWindow> window);

    void Tick();

    void ImGuiDebugPanel();

    std::shared_ptr<GLFWWindow> GetRawWindow()
    {
        return m_window;
    }

private:
    std::shared_ptr<GLFWWindow> m_window;

    uint16_t m_camera_command;
    bool     m_is_focus_mode{false};

    float  m_cursor_delta_x{0.0};
    float  m_cursor_delta_y{0.0};
    double m_last_cursor_x{0.0};
    double m_last_cursor_y{0.0};

    void onKey(int key, int scancode, int action, int mods);

    void onCursorPos(double current_cursor_x, double current_cursor_y);

    void onMouseButton(int button, int action, int mods);

    void clearState();
};

#endif //XEXAMPLE_INPUT_SYSTEM_H
