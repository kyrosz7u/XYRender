//
// Created by kyros on 2023/6/6.
//

#include "scene/camera.h"
#include "scene/scene_manager.h"

using namespace Scene;

void Camera::PostInitialize()
{
    auto scene_ptr = m_p_parent_scene.lock();

    if (scene_ptr)
    {
        render_in_scene = scene_ptr->m_render;
    } else
    {
        throw std::runtime_error("Camera::postInitialize() failed to get scene_ptr");
    }
}

void Camera::Tick()
{
    const auto render_ptr = render_in_scene.lock();
    if (render_ptr == nullptr) return;

    auto rotMat = getRotationMatrix(rotation);
    Right   = rotMat * Vector3(1, 0, 0);
    Up      = rotMat * Vector3(0, 1, 0);
    Forward = rotMat * Vector3(0, 0, 1);

    if (InputSystem.Focused)
    {
        float speed   = InputSystem.SpeedUp ? 10.0f : 5.0f;
        auto  moveDir = rotMat * InputSystem.Move;
        position = position + moveDir * speed * render_ptr->getFrameTime();

        rotation.x -= InputSystem.Look.y * render_ptr->getFrameTime() * speed * 2;
        rotation.y -= InputSystem.Look.x * render_ptr->getFrameTime() * speed * 2;

        if (rotation.x > 89.0f)
            rotation.x = 89.0f;
        if (rotation.x < -89.0f)
            rotation.x = -89.0f;
        if (rotation.y > 360.0f)
            rotation.y -= 360.0f;
        if (rotation.y < -360.0f)
            rotation.y += 360.0f;
    }
}

void Camera::ImGuiDebugPanel()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("MainCamera"))
    {
        ImGui::Text("position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
        ImGui::Text("rotation: %.2f, %.2f, %.2f", rotation.x, rotation.y, rotation.z);
        ImGui::Text("Right: %.2f, %.2f, %.2f", Right.x, Right.y, Right.z);
        ImGui::Text("Up: %.2f, %.2f, %.2f", Up.x, Up.y, Up.z);
        ImGui::Text("Forward: %.2f, %.2f, %.2f", Forward.x, Forward.y, Forward.z);
        ImGui::Text("mode: %s", mode == orthogonal ? "orthogonal" : "perspective");
        ImGui::Text("aspect: %.1f fov: %.1f", aspect, fov);
        ImGui::Text("znear: %.1f zfar: %.1f", znear, zfar);
        ImGui::TreePop();
    }
}




