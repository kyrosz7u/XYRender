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
    m_frustum_sphere = sphere;
    Vector3 light_dir = transform.GetForward();
    Vector3 light_up  = transform.GetUp();
    Vector3 light_pos = Vector3(sphere.x, sphere.y, sphere.z) - light_dir * sphere.w;


    auto r_inverse   = getRotationMatrix(transform.rotation).transpose();
    auto t_inverse   = Matrix4x4::getTrans(-light_pos);
    light_view_matrix = r_inverse * t_inverse;
    light_proj_matrix      = Math::Matrix4x4::makeOrthogonalMatrix(
            2*sphere.w, 2*sphere.w, 0.0f, sphere.w*2);
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

