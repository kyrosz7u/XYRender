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
            render->setCameraMatrix();
        }

        void Tick()
        { render->Tick(); }

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
