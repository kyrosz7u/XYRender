//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_RESOURCE_H
#define XEXAMPLE_RENDER_RESOURCE_H

#include "render/resource/render_mesh.h"

namespace RenderSystem
{
    class RenderGlobalResource
    {
    public:
        VkBuffer global_model_ubo_buffer;
        VkDeviceMemory global_model_ubo_buffer_memory;
    };

}

#endif //XEXAMPLE_RENDER_RESOURCE_H
