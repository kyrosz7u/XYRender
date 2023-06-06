//
// Created by kyrosz7u on 2023/6/6.
//

#include "core/math/math.h"
#include "transform.h"

#ifndef XEXAMPLE_LIGHT_H
#define XEXAMPLE_LIGHT_H
namespace Scene
{
    class DirectionLight
    {
    public:
        Math::Color color;
        Transform transform;

    public:
        DirectionLight() {}
        ~DirectionLight() {}
    };
}

#endif //XEXAMPLE_LIGHT_H
