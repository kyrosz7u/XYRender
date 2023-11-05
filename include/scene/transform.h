//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_TRANSFORM_H
#define XEXAMPLE_TRANSFORM_H

#include "core/math/math.h"

using namespace Math;

class Transform
{
public:
    Position position;
    Rotation rotation;
    Scale scale;
private:
    Matrix4x4 transform_matrix;
    Matrix4x4 rotation_matrix;
    Matrix4x4 scale_matrix;
    Matrix4x4 view_transform_matrix;
    Vector3 right;
    Vector3 up;
    Vector3 forward;

 public:
    Transform()
    {
        position = Position(0, 0, 0);
        rotation = Rotation(0, 0, 0);
        scale    = Scale(1, 1, 1);
    }

    Transform(Position pos, Rotation rot, Scale sca)
    {
        position = pos;
        rotation = rot;
        scale    = sca;
    }

    void Tick()
    {
        transform_matrix = Matrix4x4::IDENTITY;
        rotation_matrix = Math::getRotationMatrix(rotation);
        scale_matrix = Matrix4x4::getScale(scale);

        transform_matrix = scale_matrix * transform_matrix;
        transform_matrix = rotation_matrix * transform_matrix;
        transform_matrix = Matrix4x4::getTrans(position)*transform_matrix;

        right   = rotation_matrix * Vector3(1, 0, 0);
        up      = rotation_matrix * Vector3(0, 1, 0);
        forward = rotation_matrix * Vector3(0, 0, 1);

        view_transform_matrix = getRotationMatrix(rotation).transpose()*Matrix4x4::getTrans(-position);

    }

    Matrix4x4 GetTransformMatrix() const
    {
        return transform_matrix;
    }

    Matrix4x4 GetViewTransformMatrix() const
    {
        return view_transform_matrix;
    }

    Matrix4x4 GetRotationMatrix() const
    {
        return rotation_matrix;
    }

    Vector3 GetRight() const
    {
        return right;
    }

    Vector3 GetUp() const
    {
        return up;
    }

    Vector3 GetForward() const
    {
        return forward;
    }
};

#endif //XEXAMPLE_TRANSFORM_H
