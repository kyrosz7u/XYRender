//
// Created by kyrosz7u on 2023/10/17.
//

#include "render/shadow.h"

void DirLightShadow::UpdateShadowData(const Camera &camera)
{
    assert(m_direction_light != nullptr);

    int     atlas_side = sqrt(m_cascade_count);
    Vector2 atlas_size = Vector2(m_shadowmap_size.x / atlas_side, m_shadowmap_size.y / atlas_side);

    float current_min_distance = m_direction_light->min_shadow_distance;

    int i = 0;
    for (; i < m_cascade_count - 1; ++i)
    {
        float current_max_distance = current_min_distance + (m_direction_light->cascade_ratio[i] + 0.0001f) *
                                                            (m_direction_light->max_shadow_distance -
                                                             current_min_distance);
        m_cascade_distance[i] = Vector2(m_direction_light->min_shadow_distance, current_max_distance);
        current_min_distance = current_max_distance;

        int x = i % atlas_side;
        int y = i / atlas_side;

        // (x,y,w,h)
        m_cascade_viewport[i] = Vector4(x * atlas_size.x, y * atlas_size.y, atlas_size.x, atlas_size.y);

        camera.GetFrustumSphere(m_cascade_frustum_sphere[i], m_cascade_distance[i].x, m_cascade_distance[i].y);
        ComputeDirectionalShadowMatrices(i, atlas_side, Vector2(x, y), atlas_size, m_cascade_frustum_sphere[i]);
    }

    int x = i % atlas_side;
    int y = i / atlas_side;

    m_cascade_distance[i] = Vector2(m_direction_light->min_shadow_distance, m_direction_light->max_shadow_distance);
    m_cascade_viewport[i] = Vector4(x * atlas_size.x, y * atlas_size.y, atlas_size.x, atlas_size.y);
    camera.GetFrustumSphere(m_cascade_frustum_sphere[i], m_cascade_distance[i].x, m_cascade_distance[i].y);
    ComputeDirectionalShadowMatrices(i, atlas_side, Vector2(x, y), atlas_size, m_cascade_frustum_sphere[i]);
}

void DirLightShadow::ComputeDirectionalShadowMatrices(int cascade_index,
                                                      int atlas_side,
                                                      const Vector2 &offset,
                                                      const Vector2 &atlas_size,
                                                      const Vector4 &sphere)
{
    assert(m_direction_light != nullptr);

    const Vector3 &light_dir   = m_direction_light->transform.GetForward();
    const Vector3 &light_up    = m_direction_light->transform.GetUp();
    const Vector3 &light_right = m_direction_light->transform.GetRight();

    Vector3 &back_dummy_light_pos = m_back_dummy_light_pos_list[cascade_index];
    Vector2 pixel_size            = 2 * Vector2(sphere.w / atlas_size.x, sphere.w / atlas_size.y);
    Vector3 cur_dummy_light_pos   = Vector3(sphere.x, sphere.y, sphere.z) - light_dir * sphere.w;
    Vector3 move_dir              = cur_dummy_light_pos - back_dummy_light_pos;

    float move_x = move_dir.dotProduct(light_right);
    float move_y = move_dir.dotProduct(light_up);
    float move_z = move_dir.dotProduct(light_dir);

    if (abs(move_x) > pixel_size.x)
    {
        float size = abs(move_x) / pixel_size.x;
        size = ceil(size);

        float move = move_x > 0 ? pixel_size.x : -pixel_size.x;
        back_dummy_light_pos = back_dummy_light_pos + light_right * size * move;
    }

    if (abs(move_y) > pixel_size.y)
    {
        float size = abs(move_y) / pixel_size.y;
        size = ceil(size);

        float move = move_y > 0 ? pixel_size.y : -pixel_size.y;
        back_dummy_light_pos = back_dummy_light_pos + light_up * size * move;
    }

    back_dummy_light_pos = back_dummy_light_pos + light_dir * move_z;

    auto t_inverse = Matrix4x4::getTrans(-back_dummy_light_pos);
    auto r_inverse = getRotationMatrix(m_direction_light->transform.rotation).transpose();

    Matrix4x4 light_view_matrix = r_inverse * t_inverse;
    Matrix4x4 light_proj_matrix = Math::Matrix4x4::makeOrthogonalMatrix(
            2 * sphere.w, 2 * sphere.w, 0.1f, sphere.w * 2);

    m_cascade_viewproj_matrix[cascade_index] = light_proj_matrix * light_view_matrix;
    Matrix4x4 &m = m_cascade_sample_matrix[cascade_index];
    m = m_cascade_viewproj_matrix[cascade_index];

    float scale = 1.0f / atlas_side;

    m[0][0] = (0.5f * (m[0][0] + m[3][0]) + offset.x * m[3][0]) * scale;
    m[0][1] = (0.5f * (m[0][1] + m[3][1]) + offset.x * m[3][1]) * scale;
    m[0][2] = (0.5f * (m[0][2] + m[3][2]) + offset.x * m[3][2]) * scale;
    m[0][3] = (0.5f * (m[0][3] + m[3][3]) + offset.x * m[3][3]) * scale;

    m[1][0] = (0.5f * (m[1][0] + m[3][0]) + offset.y * m[3][0]) * scale;
    m[1][1] = (0.5f * (m[1][1] + m[3][1]) + offset.y * m[3][1]) * scale;
    m[1][2] = (0.5f * (m[1][2] + m[3][2]) + offset.y * m[3][2]) * scale;
    m[1][3] = (0.5f * (m[1][3] + m[3][3]) + offset.y * m[3][3]) * scale;

//    m[2][0] = 0.5f * (m[2][0] + m[3][0]);
//    m[2][1] = 0.5f * (m[2][1] + m[3][1]);
//    m[2][2] = 0.5f * (m[2][2] + m[3][2]);
//    m[2][3] = 0.5f * (m[2][3] + m[3][3]);
}

