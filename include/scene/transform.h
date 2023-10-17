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

    }

    Matrix4x4 GetTransformMatrix()
    {
        Matrix4x4 model_matrix = Matrix4x4::IDENTITY;
        model_matrix = Matrix4x4::getScale(scale)*model_matrix;
        model_matrix = Math::getRotationMatrix(rotation) * model_matrix;
        model_matrix = Matrix4x4::getTrans(position)*model_matrix;
        return model_matrix;
    }

    Matrix4x4 GetViewTransformMatrix()
    {
        Matrix4x4 model_matrix = Matrix4x4::IDENTITY;

        auto t_inverse   = Matrix4x4::getTrans(-position);
        auto r_inverse   = getRotationMatrix(rotation).transpose();

        model_matrix = r_inverse * t_inverse;

        return model_matrix;
    }

    Vector3 GetRight() const
    {
        return Math::getRotationMatrix(rotation) * Vector3(1, 0, 0);
    }

    Vector3 GetUp() const
    {
        return Math::getRotationMatrix(rotation) * Vector3(0, 1, 0);
    }

    Vector3 GetForward() const
    {
        return Math::getRotationMatrix(rotation) * Vector3(0, 0, 1);
    }
};

#endif //XEXAMPLE_TRANSFORM_H
