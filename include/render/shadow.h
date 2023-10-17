//
// Created by kyrosz7u on 2023/10/17.
//

#ifndef XYRENDER_SHADOW_H
#define XYRENDER_SHADOW_H

#include "core/math/math.h"
#include "scene/camera.h"
#include "scene/direction_light.h"

#include <vector>

using namespace Math;
using namespace Scene;

namespace RenderSystem
{
    class DirLightShadow
    {
    public:
        DirLightShadow(const Vector2 &shadowmap_size,
                       const std::vector<float> &cascade_ratio)
                : m_shadowmap_size(shadowmap_size),
                m_cascade_ratio(cascade_ratio)
        {
            m_cascade_view_matrix.resize(cascade_ratio.size());
        }


        ~DirLightShadow() = default;

        void Update();

        void ComputeDirectionalShadowMatrices(
                int cascade_index,
                const Camera &camera,
                const DirectionLight &light,
                Matrix4x4 &light_view_matrix,
                Matrix4x4 &light_proj_matrix);

    public:
        float                  m_min_distance   = 0.1f;
        float                  m_max_distance   = 100.0f;
        Vector2                m_shadowmap_size = Vector2(1024, 1024);
        std::vector<float>     m_cascade_ratio;
        std::vector<Matrix4x4> m_cascade_view_matrix;
    private:
        std::vector<Vector3> m_back_dummy_light_pos_list;
    };
}

#endif //XYRENDER_SHADOW_H
