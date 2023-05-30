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
            render->setCameraMatrix(getViewMatrix(), getPerspectiveMatrix());
        }

        void Tick()
        {

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
            if(mode == orthogonal)
                return Matrix4x4::makeOrthogonalMatrix(width, height, zNear, zFar, zNear, zFar);
            else
                return Matrix4x4::makePerspectiveMatrix(fov, aspect, zNear, zFar);
        }

    public:
        Vector3    position;
        EulerAngle rotation;
        CameraMode mode;
        float      width;
        float      height;
        float      aspect;
        float      fov;
        float      zNear;
        float      zFar;

        std::shared_ptr<MainCameraRender> render;
//    std::vector<VulkanAPI::RenderBase::ImageAttachment> renderTarget;
    };
}


#endif //XEXAMPLE_CAMERA_H
