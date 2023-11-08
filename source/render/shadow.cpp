//
// Created by kyrosz7u on 2023/10/17.
//

#include "render/shadow.h"

void DirLightShadow::UpdateShadowData(const Camera &camera)
{
    assert(m_direction_light != nullptr);

    int     atlas_side = sqrt(m_cascade_count);
    Vector2 atlas_size = Vector2(m_shadowmap_size.x / atlas_side, m_shadowmap_size.y / atlas_side);

    float min_distance = m_direction_light->min_shadow_distance;
    float max_distance = m_direction_light->max_shadow_distance;

    // use ue4's method to calculate cascade ratio
    float current_scale = 1.0f;
    float total_scale = 0.0f;

    std::vector<float> cascade_ratio(m_cascade_count);

    for (int i = 0; i < m_cascade_count; ++i)
    {
        cascade_ratio[i] = current_scale;
        total_scale += current_scale;
        current_scale*= m_direction_light->cascade_exponent;
    }

    for (int i=0; i < m_cascade_count; ++i)
    {
        float current_max_distance = min_distance + (max_distance - min_distance) * cascade_ratio[i] / total_scale;
        m_cascade_distance[i] = Vector2(min_distance, current_max_distance);
        min_distance = current_max_distance;

        int x = i % atlas_side;
        int y = i / atlas_side;

        // (x,y,w,h)
        m_cascade_viewport[i] = Vector4(x * atlas_size.x, y * atlas_size.y, atlas_size.x, atlas_size.y);

        camera.GetFrustumSphere(m_cascade_frustum_sphere[i], m_cascade_distance[i].x, m_cascade_distance[i].y);
        ComputeDirectionalShadowMatrices(i, atlas_side, Vector2(x, y), atlas_size, m_cascade_frustum_sphere[i]);
    }
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

    const Matrix4x4 &light_view_matrix = m_direction_light->transform.GetViewTransformMatrix();
    const Vector4 center_pos = Vector4(sphere.x, sphere.y, sphere.z, 1.0f);
    const Vector4 sphere_in_light_space = light_view_matrix * center_pos;

    const int max_downsample = 4;
    float snap_x = ::fmod(sphere_in_light_space.x, max_downsample*pixel_size.x);
    float snap_y = ::fmod(sphere_in_light_space.y, max_downsample*pixel_size.y);


    //snap_x = 0.0f;
    //snap_y = 0.0f;

    const Matrix4x4 light_view_inverse = light_view_matrix.inverse();
    const Vector4 snapped_center_pos = light_view_inverse * Vector4(sphere_in_light_space.x - snap_x, sphere_in_light_space.y - snap_y, sphere_in_light_space.z, 1.0f);

    back_dummy_light_pos = snapped_center_pos.xyz() - 2.0f*light_dir * sphere.w;

    auto t_inverse = Matrix4x4::getTrans(-back_dummy_light_pos);
    auto r_inverse = m_direction_light->transform.GetRotationMatrix().transpose();

    Matrix4x4 light_proj_matrix = Math::Matrix4x4::makeOrthogonalMatrix(
            2 * sphere.w, 2 * sphere.w, 0.1f, sphere.w * 3.0f);

    m_cascade_viewproj_matrix[cascade_index] = light_proj_matrix * r_inverse * t_inverse;;
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

    //Vector3 center = Vector3(sphere.x, sphere.y, sphere.z);
    //if(back_dummy_light_pos.distance(center) > 0.1f + sphere.w)
    //{
    //    std::cout << "distance: " << back_dummy_light_pos.distance(center) << std::endl;
    //}
}

void DirLightShadow::ImGuiDebugPanel()
{
    ImGui::Begin("Directional Light Shadow[0]");
    ImGui::Text("Light Position: %f, %f, %f", m_direction_light->transform.position.x,
                m_direction_light->transform.position.y,
                m_direction_light->transform.position.z);
    ImGui::Text("Light Direction: %f, %f, %f", m_direction_light->transform.GetForward().x,
                m_direction_light->transform.GetForward().y,
                m_direction_light->transform.GetForward().z);
    ImGui::Text("Light Up: %f, %f, %f", m_direction_light->transform.GetUp().x,
                m_direction_light->transform.GetUp().y,
                m_direction_light->transform.GetUp().z);
    ImGui::Text("Light Right: %f, %f, %f", m_direction_light->transform.GetRight().x,
                m_direction_light->transform.GetRight().y,
                m_direction_light->transform.GetRight().z);

    ImGui::Text("Project Matrix: ");
    ImGui::Text("%f, %f, %f, %f", m_cascade_viewproj_matrix[0][0][0], m_cascade_viewproj_matrix[0][0][1],
                m_cascade_viewproj_matrix[0][0][2], m_cascade_viewproj_matrix[0][0][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_viewproj_matrix[0][1][0], m_cascade_viewproj_matrix[0][1][1],
                m_cascade_viewproj_matrix[0][1][2], m_cascade_viewproj_matrix[0][1][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_viewproj_matrix[0][2][0], m_cascade_viewproj_matrix[0][2][1],
                m_cascade_viewproj_matrix[0][2][2], m_cascade_viewproj_matrix[0][2][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_viewproj_matrix[0][3][0], m_cascade_viewproj_matrix[0][3][1],
                m_cascade_viewproj_matrix[0][3][2], m_cascade_viewproj_matrix[0][3][3]);

    ImGui::Text("Sample Matrix: ");
    ImGui::Text("%f, %f, %f, %f", m_cascade_sample_matrix[0][0][0], m_cascade_sample_matrix[0][0][1],
                m_cascade_sample_matrix[0][0][2], m_cascade_sample_matrix[0][0][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_sample_matrix[0][1][0], m_cascade_sample_matrix[0][1][1],
                m_cascade_sample_matrix[0][1][2], m_cascade_sample_matrix[0][1][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_sample_matrix[0][2][0], m_cascade_sample_matrix[0][2][1],
                m_cascade_sample_matrix[0][2][2], m_cascade_sample_matrix[0][2][3]);
    ImGui::Text("%f, %f, %f, %f", m_cascade_sample_matrix[0][3][0], m_cascade_sample_matrix[0][3][1],
                m_cascade_sample_matrix[0][3][2], m_cascade_sample_matrix[0][3][3]);


    ImGui::End();
}

