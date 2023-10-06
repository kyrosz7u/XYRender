//
// Created by kyros on 2023/6/6.
//

#include "scene/camera.h"
#include "scene/scene_manager.h"
#include "render/render_base.h"

using namespace Scene;
using namespace RenderSystem;

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

//    auto rotMat = getRotationMatrix(rotation);
//    Right   = rotMat * Vector3(1, 0, 0);
//    Up      = rotMat * Vector3(0, 1, 0);
//    Forward = rotMat * Vector3(0, 0, 1);

    auto rotMat = Matrix3x3(Right, Up, Forward);
    rotMat = rotMat.transpose();

    if (InputSystem.Focused)
    {
        float speed   = InputSystem.SpeedUp ? 10.0f : 5.0f;
        auto  moveDir = rotMat * InputSystem.Move;
        position = position + moveDir * speed * render_ptr->getFrameTime();

        rotation.x += InputSystem.Look.y * render_ptr->getFrameTime() * speed * 2;
        rotation.y -= InputSystem.Look.x * render_ptr->getFrameTime() * speed * 2;

        if (rotation.x > 89.0f)
            rotation.x = 89.0f;
        if (rotation.x < -89.0f)
            rotation.x = -89.0f;
        if (rotation.y > 360.0f)
            rotation.y -= 360.0f;
        if (rotation.y < -360.0f)
            rotation.y += 360.0f;

        Forward.x = cos(rotation.x * Math_PI / 180.0f) * sin(rotation.y * Math_PI / 180.0f);
        Forward.y = sin(rotation.x * Math_PI / 180.0f);
        Forward.z = cos(rotation.x * Math_PI / 180.0f) * cos(rotation.y * Math_PI / 180.0f);

        Forward.normalise();

        auto WorldUp = Vector3(0, 1, 0);
        Right = WorldUp.crossProduct(Forward);
        Right.normalise();
        Up = Forward.crossProduct(Right);
        Up.normalise();
    }
}

// https://lxjk.github.io/2017/04/15/Calculate-Minimal-Bounding-Sphere-of-Frustum.html
void Camera::GetFrustumSphere(Vector4 &spere, float max_distance, float near_bias = 0.0f) const
{
    float far_plane = max_distance > 0 && max_distance < zfar ? max_distance : zfar;
    float near_plane = znear + near_bias;
    near_plane = near_plane > 0.0f && near_plane < far_plane ? near_plane : znear;
    Vector3 center;
    float   radius;
    float k = sqrt(1.0f + 1.0f/(aspect * aspect)) * tan(fov / 2.0f);
    float k2 = k * k;
    float k4 = k2 * k2;

    if(k2>=(far_plane-near_plane)/(far_plane+near_plane))
    {
        center = Vector3(0, 0, far_plane);
        radius = far_plane * k;
    }
    else
    {
        center = Vector3(0, 0, (far_plane+near_plane)*(1+k2)/2.0f);
        radius = 0.5f * sqrt((far_plane-near_plane)*(far_plane-near_plane) + 2.0f*(far_plane*far_plane+near_plane*near_plane)*k2 + (far_plane+near_plane)*(far_plane+near_plane)*k4);
    }

    spere = Vector4(position + center*Forward, radius);
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




