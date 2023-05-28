//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_UBO_H
#define XEXAMPLE_RENDER_UBO_H

#include "math/Math.h"

namespace RenderSystem
{
    struct VulkanUBOModel
    {
        Math::Matrix4x4 model;
    };
    struct VulkanUBOViewProjection
    {
        Math::Matrix4x4 view;
        Math::Matrix4x4 proj;
    };

}
#endif //XEXAMPLE_RENDER_UBO_H
