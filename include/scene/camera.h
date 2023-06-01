//
// Created by kyrosz7u on 2023/5/17.
//

#ifndef XEXAMPLE_CAMERA_H
#define XEXAMPLE_CAMERA_H

#include "math/Math.h"
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
            render = std::make_shared<MainCameraRender>();
            render->initialize();
        }

        void Tick()
        {
            render->setCameraMatrix(getViewMatrix(), getPerspectiveMatrix());
            render->Tick();
        }

        Matrix4x4 getViewMatrix()
        {
            auto r_inverse = Matrix4x4::getRotation(rotation);
            r_inverse = r_inverse.transpose();
            auto t_inverse = Matrix4x4::getTrans(-position);

            return r_inverse * t_inverse;
        }

        Matrix4x4 getPerspectiveMatrix()
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

    public:
        Vector3    position;
        EulerAngle rotation;
        CameraMode mode;
        float      width;
        float      height;
        float      aspect;
        float      fov;
        float      znear;
        float      zfar;

        std::shared_ptr<MainCameraRender> render;
        std::function<void(int, int)>     windowSizeChangedDelegate = std::bind(
                &Camera::windowSizeChangedHandler, this,
                std::placeholders::_1,
                std::placeholders::_2);
//    std::vector<VulkanAPI::RenderBase::ImageAttachment> renderTarget;
    };
}


#endif //XEXAMPLE_CAMERA_H
