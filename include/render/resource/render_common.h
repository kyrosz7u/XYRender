//
// Created by kyros on 5/25/23.
//

#ifndef XEXAMPLE_RENDER_COMMON_H
#define XEXAMPLE_RENDER_COMMON_H

#include "math/Math.h"
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <memory>

using namespace Math;

namespace RenderSystem
{
    enum TEXTURE_TYPE : uint8_t
    {
        VULKAN_TEXTURE_UNDEFINE = 0,
        VULKAN_TEXTURE_2D,
        VULKAN_TEXTURE_CUBE
    };

    class RawBufferData
    {
    public:
        size_t m_size{0};
        void   *m_data{nullptr};

        RawBufferData() = delete;

        RawBufferData(size_t size)
        {
            m_size = size;
            m_data = malloc(size);
        }

        ~RawBufferData()
        {
            if (m_data)
            {
                free(m_data);  // 内存碎片整理依赖操作系统的堆内存管理机制
            }
        }

        bool isValid() const
        { return m_data != nullptr; }
    };

    class TextureBufferData
    {
    public:
        uint32_t m_width{0};
        uint32_t m_height{0};
        uint32_t m_depth{0};
        uint32_t m_mip_levels{0};
        uint32_t m_array_layers{0};
        void     *m_pixels{nullptr};

        VkFormat     m_format = VK_FORMAT_UNDEFINED;
        TEXTURE_TYPE m_type{VULKAN_TEXTURE_UNDEFINE};

        TextureBufferData() = default;

        ~TextureBufferData()
        {
            if (m_pixels)
            {
                free(m_pixels);
            }
        }

        bool isValid() const
        { return m_pixels != nullptr; }
    };

    struct MeshData
    {
        std::shared_ptr<RawBufferData> m_vertex_buffer;
        std::shared_ptr<RawBufferData> m_index_buffer;
    };

    struct MeshVertexDataDefinition
    {
        Vector3 pos;       // position
        Vector3 normal;    // normal
        Vector4 tangent;   // tangent
        Vector2 texcoord;  // UV coordinates
    };
}
#endif  //XEXAMPLE_RENDER_COMMON_H
