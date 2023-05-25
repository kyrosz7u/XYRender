#include "math/Math.h"
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <memory>

using namespace Math;

namespace Vulkan
{
    enum TEXTURE_TYPE: uint8_t
    {
        VULKAN_TEXTURE_UNDEFINE = 0,
        VULKAN_TEXTURE_2D,
        VULKAN_TEXTURE_CUBE
    };

    class BufferData
    {
    public: 
        size_t m_size {0};
        void*  m_data {nullptr};

        BufferData() = delete;
        BufferData(size_t size)
        {
            m_size = size;
            m_data = malloc(size);
        }
        ~BufferData()
        {
            if (m_data)
            {
                free(m_data);  // 内存碎片整理依赖操作系统的堆内存管理机制
            }
        }
        bool isValid() const { return m_data != nullptr; }
    };

    class TextureData
    {
    public: 
        uint32_t m_width {0};
        uint32_t m_height {0};
        uint32_t m_depth {0};
        uint32_t m_mip_levels {0};
        uint32_t m_array_layers {0};
        void*    m_pixels {nullptr};

        VkFormat m_format = VK_FORMAT_UNDEFINED;
        TEXTURE_TYPE   m_type {VULKAN_TEXTURE_UNDEFINE };

        TextureData() = default;
        ~TextureData()
        {
            if (m_pixels)
            {
                free(m_pixels);
            }
        }
        bool isValid() const { return m_pixels != nullptr; }
    };

    struct MeshData
    {
        std::shared_ptr<BufferData> m_vertex_buffer;
        std::shared_ptr<BufferData> m_index_buffer;
    };

    struct MeshVertexDataDefinition
    {
        Vector3 x,  y,  z;       // position
        float nx, ny, nz;      // normal
        float tx, ty, tz, td;  // tangent
        float u,  v;           // UV coordinates
    };

};