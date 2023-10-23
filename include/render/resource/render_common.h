//
// Created by kyros on 5/25/23.
//

#ifndef XEXAMPLE_RENDER_COMMON_H
#define XEXAMPLE_RENDER_COMMON_H

#include "core/math/math.h"
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <memory>

#define MAX_DIRECTIONAL_LIGHT_COUNT 16

using namespace Math;

namespace RenderSystem
{
    struct VulkanModelDefine
    {
        Math::Matrix4x4 model;
        Math::Matrix4x4 normal;
    };

    struct VulkanPerFrameSceneDefine
    {
        Math::Matrix4x4 proj_view;
        Math::Vector3   camera_pos;
        uint32_t        directional_light_number;
    };

    struct VulkanPerFrameDirectionalLightDefine
    {
        Math::Color   color;
        Math::Vector3 direction;
        float         intensity;
        int           cascade_count;
    };

    struct VulkanLightProjectDefine
    {
        Math::Matrix4x4 light_proj;
    };

    struct VulkanShadowMapSampleDataDefine
    {
        Math::Matrix4x4 light_proj;
        Math::Vector4   light_frustum_sphere;
    };
}
#endif  //XEXAMPLE_RENDER_COMMON_H
