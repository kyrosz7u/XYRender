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
                       const DirectionLight &direction_light,
                       int light_index)
                : m_shadowmap_size(shadowmap_size),
                  m_direction_light(&direction_light),
                  m_light_index(light_index)
        {
            m_cascade_count = direction_light.GetCascadeRatio().size() + 1;
            m_cascade_distance.resize(m_cascade_count);
            m_cascade_viewport.resize(m_cascade_count);
            m_cascade_frustum_sphere.resize(m_cascade_count);
            m_cascade_viewproj_matrix.resize(m_cascade_count);
            m_cascade_sample_matrix.resize(m_cascade_count);
            m_back_dummy_light_pos_list.resize(m_cascade_count);
        }

        ~DirLightShadow() = default;

        void UpdateShadowData(const Camera &camera);

        int GetLightIndex() const
        {
            return m_light_index;
        }

    private:
        void ComputeDirectionalShadowMatrices(int cascade_index,
                                              int atlas_side,
                                              const Vector2 &offset,
                                              const Vector2 &atlas_size,
                                              const Vector4 &sphere);

    public:
        int                    m_cascade_count;
        std::vector<Vector2>   m_cascade_distance;
        std::vector<Vector4>   m_cascade_viewport;
        std::vector<Vector4>   m_cascade_frustum_sphere;
        std::vector<Matrix4x4> m_cascade_viewproj_matrix;
        std::vector<Matrix4x4> m_cascade_sample_matrix;
    private:
        int                  m_light_index      = 0;
        const DirectionLight *m_direction_light = nullptr;
        Vector2              m_shadowmap_size;
        std::vector<Vector3> m_back_dummy_light_pos_list;


    };
}

#endif //XYRENDER_SHADOW_H
