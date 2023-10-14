//
// Created by kyrosz7u on 2023/6/6.
//

#include "core/math/math.h"
#include "transform.h"
#include "camera.h"
#include "render/render_base.h"

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

            void SetRenderContext(std::shared_ptr<RenderBase> render_context)
            {
                m_render_context = render_context;
            }

            void ComputeDirectionalShadowMatrices(
                    const Camera &camera,
                    Vector4 &sphere,
                    Matrix4x4 &light_view_matrix,
                    Matrix4x4 &light_proj_matrix);
            void ImGuiDebugPanel();
        private:
            Vector4 m_frustum_sphere;
            Vector4 m_sphere_backup;
            Vector2 m_shadowmap_size;
            std::shared_ptr<RenderBase> m_render_context;

            bool IsMatrixNeedUpdate(const Vector4 &, const Vector4 &, const Vector3 &, const Vector3 &);
    };
}

#endif //XEXAMPLE_LIGHT_H
