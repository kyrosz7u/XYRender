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
    };

    struct VulkanSkyboxCameraDefine
    {
        Math::Matrix4x4 model;
    };
}
#endif  //XEXAMPLE_RENDER_COMMON_H
