//
// Created by kyrosz7u on 2023/6/6.
//

#include "core/math/math.h"
#include "transform.h"
#include "camera.h"

#ifndef XEXAMPLE_LIGHT_H
#define XEXAMPLE_LIGHT_H

namespace Scene
{
    class Camera;
    class DirectionLight
    {
        public:
            float intensity;
            Math::Color color;
            Transform transform;

        public:
            DirectionLight()
            {}
            ~DirectionLight() {}

            void Tick();

            void ComputeDirectionalShadowMatrices(
                    const Camera &camera,
                    Vector4 &sphere,
                    Matrix4x4 &light_view_matrix,
                    Matrix4x4 &light_proj_matrix);
            void ImGuiDebugPanel();
    };
}

#endif //XEXAMPLE_LIGHT_H
