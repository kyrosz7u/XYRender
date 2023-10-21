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
            float min_shadow_distance;
            float max_shadow_distance;
            std::vector<float> cascade_ratio;

        public:
            DirectionLight()
            {}
            ~DirectionLight() {}

            void Tick();

            const std::vector<float>& GetCascadeRatio() const
            {
                return cascade_ratio;
            }

            const Vector2& GetMaxShadowDistance() const
            {
                return Vector2(min_shadow_distance, max_shadow_distance);
            }

            void ComputeDirectionalShadowMatrices(
                    const Camera &camera,
                    Vector4 &sphere,
                    Matrix4x4 &light_view_matrix,
                    Matrix4x4 &light_proj_matrix) const;
            void ImGuiDebugPanel();
    };
}

#endif //XEXAMPLE_LIGHT_H
