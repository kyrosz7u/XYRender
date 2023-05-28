//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_RESOURCE_H
#define XEXAMPLE_RENDER_RESOURCE_H

#include "render/resource/render_mesh.h"

namespace RenderSystem
{
    class RenderResource
    {
    public:
        std::vector<RenderMesh> m_meshes;
    };

}

#endif //XEXAMPLE_RENDER_RESOURCE_H
