//
// Created by kyrosz7u on 2023/5/17.
//

#ifndef XEXAMPLE_CAMERA_H
#define XEXAMPLE_CAMERA_H

#include "math/Math.h"
#include "render/MainCameraRender.h"

using namespace Math;

enum CameraMode
{
    orthogonal,
    perspective
};

class Camera
{
public:
    void Tick(){render->Tick(); }

public:
    Vector3 position;
    EulerAngle rotation;

    float width;
    float height;
    float aspect;
    float fov;
    float zNear;
    float zFar;

private:
    std::shared_ptr<RenderBases> render;
    std::vector<RenderBases::ImageAttachment> renderTarget;
};

#endif //XEXAMPLE_CAMERA_H
