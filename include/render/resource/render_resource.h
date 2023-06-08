//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_RESOURCE_H
#define XEXAMPLE_RENDER_RESOURCE_H


#include "core/graphic/vulkan/vulkan_buffer.h"
#include "render/resource/render_ubo.h"
#include "render_mesh.h"
#include "ui/ui_overlay.h"
#include "render_texture.h"
#include <memory>

using namespace VulkanAPI;

namespace RenderSystem
{
    struct RenderGlobalResourceInfo
    {
        std::vector<RenderSubmesh> *p_visible_submeshes;
        std::vector<Texture2DPtr>  *p_visible_textures;
        RenderModelUBOList         *p_render_model_ubo_list;
        RenderPerFrameUBO          *p_render_per_frame_ubo;
        std::weak_ptr<UIOverlay>   p_ui_overlay;
    };
}

#endif //XEXAMPLE_RENDER_RESOURCE_H
