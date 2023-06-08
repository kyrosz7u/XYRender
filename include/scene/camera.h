//
// Created by kyrosz7u on 2023/5/17.
//

#ifndef XEXAMPLE_CAMERA_H
#define XEXAMPLE_CAMERA_H

#include <imgui.h>

#include "core/math/math.h"
#include "input/input_system.h"
#include "render/forward_render.h"

using namespace Math;
using namespace RenderSystem;

namespace Scene
{
    enum CameraMode
    {
        orthogonal,
        perspective
    };

    class SceneManager;

    class Camera
    {
    public:
        Vector3                       position;
        EulerAngle                    rotation;
        Vector3                       Right                     = Vector3(1, 0, 0);
        Vector3                       Up                        = Vector3(0, 1, 0);
        Vector3                       Forward                   = Vector3(0, 0, 1);
        CameraMode                    mode;
        float                         width;
        float                         height;
        float                         aspect;
        float                         fov;
        float                         znear;
        float                         zfar;

        std::function<void(int, int)> windowSizeChangedDelegate = std::bind(
                &Camera::windowSizeChangedHandler, this,
                std::placeholders::_1,
                std::placeholders::_2);

    public:
        Camera(float aspect, float fov, float znear, float zfar, CameraMode mode = perspective)
                : aspect(aspect), fov(fov), znear(znear), zfar(zfar), mode(mode)
        {
            InputSystem.GetRawWindow()->registerOnWindowSizeFunc(windowSizeChangedDelegate);
        }

        void PostInitialize();

        void Tick();

        void ImGuiDebugPanel();

        void setParentScene(std::weak_ptr<SceneManager> parent_scene)
        {
            m_p_parent_scene = parent_scene;
        }

        Matrix4x4 calculateViewMatrix()
        {
            auto r_inverse = getRotationMatrix(rotation);
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


    private:
        std::weak_ptr<SceneManager>  m_p_parent_scene;
        std::weak_ptr<ForwardRender> render_in_scene;
    };
}


#endif //XEXAMPLE_CAMERA_H
