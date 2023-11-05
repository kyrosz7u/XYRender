//
// Created by 樱吹雪 on 10/6/23.
//

#include "scene/direction_light.h"
#include "scene/camera.h"

using namespace Scene;

void
DirectionLight::ComputeDirectionalShadowMatrices(const Camera &camera, Vector4 &sphere,
                                                 Matrix4x4 &light_view_matrix,
                                                 Matrix4x4 &light_proj_matrix) const
{
    throw std::runtime_error("DirectionLight::ComputeDirectionalShadowMatrices() deprecated");
    //camera.GetFrustumSphere(sphere, 20.0f, 0.0f);
    //const Vector3& light_dir = transform.GetForward();
    //const Vector3& light_up  = transform.GetUp();
    //const Vector3 &light_right = transform.GetRight();
    //
    //Vector2 pixel_size = 20*Vector2(sphere.w / m_shadowmap_size.x, sphere.w / m_shadowmap_size.y);
    //
    //Vector3 cur_light_pos = Vector3(sphere.x, sphere.y, sphere.z) - light_dir * sphere.w;
    //Vector3 move_dir = cur_light_pos - transform.position;
    //
    //float move_x = move_dir.dotProduct(light_right);
    //float move_y = move_dir.dotProduct(light_up);
    //float move_z = move_dir.dotProduct(light_dir);
    //
    //if(abs(move_x) > pixel_size.x)
    //{
    //    float size = abs(move_x) / pixel_size.x;
    //    size = ceil(size);
    //
    //    float move = move_x > 0 ? pixel_size.x : -pixel_size.x;
    //    transform.position = transform.position + light_right * size * move;
    //}
    //
    //if(abs(move_y) > pixel_size.y)
    //{
    //    float size = abs(move_y) / pixel_size.y;
    //    size = ceil(size);
    //
    //    float move = move_y > 0 ? pixel_size.y : -pixel_size.y;
    //    transform.position = transform.position + light_up * size * move;
    //}
    //
    //transform.position = transform.position + light_dir * move_z;
    //
    //m_frustum_sphere = sphere;
    //
    //auto t_inverse   = Matrix4x4::getTrans(-transform.position);
    //auto r_inverse   = getRotationMatrix(transform.rotation).transpose();
    //
    //light_view_matrix = r_inverse * t_inverse;
    //light_proj_matrix      = Math::Matrix4x4::makeOrthogonalMatrix(
    //        2*m_frustum_sphere.w, 2*m_frustum_sphere.w, 0.1f, m_frustum_sphere.w*2);
}

void DirectionLight::Tick()
{
    transform.Tick();
}

void DirectionLight::ImGuiDebugPanel()
{

}

