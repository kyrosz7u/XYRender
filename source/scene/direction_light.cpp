//
// Created by 樱吹雪 on 10/6/23.
//

#include "scene/direction_light.h"
#include "scene/camera.h"

using namespace Scene;

void
DirectionLight::ComputeDirectionalShadowMatrices(const Camera &camera, Vector4 &sphere,
                                                 Matrix4x4 &light_view_matrix,
                                                 Matrix4x4 &light_proj_matrix)
{
    camera.GetFrustumSphere(sphere, 20.0f, 0.0f);
    const Vector3& light_dir = transform.GetForward();
    const Vector3& light_up  = transform.GetUp();
    const Vector3& light_right = transform.GetRight();

    m_shadowmap_size = m_render_context->getShadowMapSize();

    Vector2 pixel_size = 20*Vector2(sphere.w / m_shadowmap_size.x, sphere.w / m_shadowmap_size.y);

    Vector3 cur_light_pos = Vector3(sphere.x, sphere.y, sphere.z) - light_dir * sphere.w;
    Vector3 move_dir = cur_light_pos - transform.position;

    float move_x = move_dir.dotProduct(light_right);
    float move_y = move_dir.dotProduct(light_up);
    float move_z = move_dir.dotProduct(light_dir);

    if(abs(move_x) > pixel_size.x)
    {
        float size = abs(move_x) / pixel_size.x;
        size = ceil(size);

        float move = move_x > 0 ? pixel_size.x : -pixel_size.x;
        transform.position = transform.position + light_right * size * move;
    }

    if(abs(move_y) > pixel_size.y)
    {
        float size = abs(move_y) / pixel_size.y;
        size = ceil(size);

        float move = move_y > 0 ? pixel_size.y : -pixel_size.y;
        transform.position = transform.position + light_up * size * move;
    }

    transform.position = transform.position + light_dir * move_z;


    m_frustum_sphere = sphere;
//
//    transform.position = Vector3(m_frustum_sphere.x, m_frustum_sphere.y, m_frustum_sphere.z)
//            - light_dir * m_frustum_sphere.w;

    auto t_inverse   = Matrix4x4::getTrans(-transform.position);
    auto r_inverse   = getRotationMatrix(transform.rotation).transpose();

    light_view_matrix = r_inverse * t_inverse;
    light_proj_matrix      = Math::Matrix4x4::makeOrthogonalMatrix(
            2*m_frustum_sphere.w, 2*m_frustum_sphere.w, 0.1f, m_frustum_sphere.w*2);
}

bool DirectionLight::IsMatrixNeedUpdate(const Vector4 &last_sphere,
                                        const Vector4 &cur_sphere,
                                        const Vector3 &light_right,
                                        const Vector3 &light_up)
{
    Vector3 move_dir = (cur_sphere - last_sphere).xyz();
    m_shadowmap_size = m_render_context->getShadowMapSize();

    Vector2 pixel_size = Vector2(last_sphere.z / m_shadowmap_size.x, last_sphere.z / m_shadowmap_size.y);

    float move_x = abs(move_dir.dotProduct(light_right));
    float move_y = abs(move_dir.dotProduct(light_up));
    float max_move = std::max(move_x, move_y);

    if (move_x > pixel_size.x || move_y > pixel_size.y)
    {
        return true;
    }
    else
    {
        return false;
    }

}

void DirectionLight::ImGuiDebugPanel()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light Sphere"))
    {
        ImGui::Text("position: %.2f, %.2f, %.2f", m_frustum_sphere.x, m_frustum_sphere.y,
                    m_frustum_sphere.z);
        ImGui::Text("radius: %.2f", m_frustum_sphere.w);
        ImGui::TreePop();
    }
}

