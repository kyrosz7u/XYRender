//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_RESOURCE_H
#define XEXAMPLE_RENDER_RESOURCE_H


#include "graphic/vulkan/vulkan_buffer.h"
#include "render/resource/render_ubo.h"
#include "render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    struct RenderGlobalResourceInfo
    {
        std::vector<RenderMeshPtr>*   p_visible_meshes;
        RenderModelUBOList*           p_render_model_ubo_list;
        RenderPerFrameUBO*            p_render_per_frame_ubo;
    };

}

#endif //XEXAMPLE_RENDER_RESOURCE_H
