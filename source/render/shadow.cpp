//
// Created by kyrosz7u on 2023/10/17.
//

#include "render/shadow.h"

void DirLightShadow::Update()
{
    for (int i = 0; i < m_cascade_ratio.size(); ++i)
    {
        m_cascade_view_matrix[i] = Matrix4x4::IDENTITY;
    }


}

void DirLightShadow::ComputeDirectionalShadowMatrices(int cascade_index,
                                                      const Camera &camera,
                                                      const DirectionLight &light,
                                                      Matrix4x4 &light_view_matrix,
                                                      Matrix4x4 &light_proj_matrix)
{
    Vector4 sphere;
    camera.GetFrustumSphere(sphere, camera.znear, 0.0f);
    const Vector3 &light_dir   = light.transform.GetForward();
    const Vector3 &light_up    = light.transform.GetUp();
    const Vector3 &light_right = light.transform.GetRight();

    Vector3 &back_dummy_light_pos = m_back_dummy_light_pos_list[cascade_index];
    Vector2 pixel_size            = 2 * Vector2(sphere.w / m_shadowmap_size.x, sphere.w / m_shadowmap_size.y);
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
    auto r_inverse = getRotationMatrix(light.transform.rotation).transpose();

    light_view_matrix = r_inverse * t_inverse;
    light_proj_matrix = Math::Matrix4x4::makeOrthogonalMatrix(
            2 * sphere.w, 2 * sphere.w, 0.1f, sphere.w * 2);
}

