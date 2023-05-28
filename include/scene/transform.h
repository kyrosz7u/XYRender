//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_TRANSFORM_H
#define XEXAMPLE_TRANSFORM_H

#include "math/Math.h"

using namespace Math;

class Transform
{
public:
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_scale;

    Matrix4x4 GetTransformMatrix()
    {
        Matrix4x4 scale_matrix = Matrix4x4::getScale(m_scale);
        Matrix4x4 rotation_matrix = Matrix4x4::getRotation(m_rotation);
        Matrix4x4 translation_matrix = Matrix4x4::getTrans(m_position);
        return translation_matrix * rotation_matrix * scale_matrix;
    }
};

#endif //XEXAMPLE_TRANSFORM_H
