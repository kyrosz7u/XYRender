//
// Created by kyrosz7u on 2023/5/17.
//

#ifndef XEXAMPLE_CAMERA_H
#define XEXAMPLE_CAMERA_H

#include "math/Math.h"

using namespace Math;

enum CameraMode
{
    orthogonal,
    perspective
};

class Camera
{
private:
    Vector3 position;
    EulerAngle rotation;

    float width;
    float height;
    float aspect;
    float fov;
    float nearZ;
    float farZ;
};

#endif //XEXAMPLE_CAMERA_H
