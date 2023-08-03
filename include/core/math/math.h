//
// Created by kyrosz7u on 2023/5/17.
//

//#ifndef XEXAMPLE_MATH_H
//#define XEXAMPLE_MATH_H

#include "matrix3x3.h"
#include "matrix4x4.h"

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

namespace Math
{
    typedef Vector3    EulerAngle; // degree unit pitch, yaw, row
    typedef Vector4    Color;
    typedef Vector2    TexCoord;
    typedef Vector3    Position;
    typedef EulerAngle Rotation;
    typedef Vector3    Scale;

    bool realEqual(float a, float b, float tolerance /* = std::numeric_limits<float>::epsilon() */);

    Matrix4x4 getRotationMatrix(const EulerAngle &eulerAngle);
}

//#endif //XEXAMPLE_MATH_H
