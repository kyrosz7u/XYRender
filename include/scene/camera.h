//
// Created by kyrosz7u on 2023/5/17.
//

#ifndef XEXAMPLE_CAMERA_H
#define XEXAMPLE_CAMERA_H

#include <imgui.h>

#include "core/math/math.h"
#include "input/input_system.h"
#include "render/main_camera_render.h"

using namespace Math;
using namespace RenderSystem;

namespace Scene
{
    enum CameraMode
    {
        orthogonal,
        perspective
    };

    class Camera
    {
    public:
        Camera()
        {
            m_render = std::make_shared<MainCameraRender>();
            m_render->initialize();
        }

        void Tick()
        {
            auto rotMat = Matrix4x4::getRotation(rotation);
            Right   = rotMat * Vector3(1, 0, 0);
            Up      = rotMat * Vector3(0, 1, 0);
            Forward = rotMat * Vector3(0, 0, 1);

            if (InputSystem.Focused)
            {
                float speed = InputSystem.SpeedUp ? 10.0f : 1.0f;
                moveDir  = rotMat * InputSystem.Move;
                position = position + moveDir * speed * m_render->getFrameTime();
            }
            m_render->setCameraMatrix(calculateViewMatrix(), calculatePerspectiveMatrix());
            m_render->Tick();
        }

        Matrix4x4 calculateViewMatrix()
        {
            auto r_inverse = Matrix4x4::getRotation(rotation);
            r_inverse = r_inverse.transpose();
            auto t_inverse = Matrix4x4::getTrans(-position);

            return r_inverse * t_inverse;
        }

        Matrix4x4 calculatePerspectiveMatrix()
        {
            if (mode == orthogonal)
                return Matrix4x4::makeOrthogonalMatrix(width, height, znear, zfar, znear, zfar);
            else
                return Matrix4x4::makePerspectiveMatrix(fov, aspect, znear, zfar);
        }

        void windowSizeChangedHandler(int width, int height)
        {
            this->width  = width;
            this->height = height;
            aspect = (float) width / height;
        }

        void ImGuiDebugPanel()
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("MainCamera"))
            {
                ImGui::Text("position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
                ImGui::Text("rotation: %.2f, %.2f, %.2f", rotation.x, rotation.y, rotation.z);
                ImGui::Text("Right: %.2f, %.2f, %.2f", Right.x, Right.y, Right.z);
                ImGui::Text("Up: %.2f, %.2f, %.2f", Up.x, Up.y, Up.z);
                ImGui::Text("Forward: %.2f, %.2f, %.2f", Forward.x, Forward.y, Forward.z);
                ImGui::Text("moveDir: %.2f, %.2f, %.2f", moveDir.x, moveDir.y, moveDir.z);
                ImGui::Text("mode: %s", mode == orthogonal ? "orthogonal" : "perspective");
                ImGui::Text("width: %.1f height: %.1f", width, height);
                ImGui::Text("aspect: %.1f fov: %.1f", aspect, fov);
                ImGui::Text("znear: %.1f zfar: %.1f", znear, zfar);
                ImGui::TreePop();
            }
        }

    public:
        Vector3    position;
        EulerAngle rotation;
        Vector3    Right   = Vector3(1, 0, 0);
        Vector3    Up      = Vector3(0, 1, 0);
        Vector3    Forward = Vector3(0, 0, 1);
        CameraMode mode;
        float      width;
        float      height;
        float      aspect;
        float      fov;
        float      znear;
        float      zfar;

        Vector3 moveDir;

        std::shared_ptr<MainCameraRender> m_render;
        std::function<void(int, int)>     windowSizeChangedDelegate = std::bind(
                &Camera::windowSizeChangedHandler, this,
                std::placeholders::_1,
                std::placeholders::_2);

    };
}


#endif //XEXAMPLE_CAMERA_H
