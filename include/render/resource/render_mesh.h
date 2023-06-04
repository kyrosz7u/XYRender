//
// Created by kyros on 5/25/23.
//

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core/graphic/vulkan/vulkan_context.h"
#include "core/math/math.h"

#include <vector>
#include <array>
#include <memory>

using namespace Math;

#ifndef XEXAMPLE_RENDER_MESH_H
#define XEXAMPLE_RENDER_MESH_H

namespace RenderSystem
{
    extern std::shared_ptr<VulkanAPI::VulkanContext> g_p_vulkan_context;
    class RenderMesh;

    typedef std::shared_ptr<RenderMesh> RenderMeshPtr;

    struct VulkanMeshVertexPostition
    {
        Vector3 position;
    };

    struct VulkanMeshVertexNormal
    {
        Vector3 normal;
        Vector4 tangent;
    };

    struct VulkanMeshVertexTexcoord
    {
        Vector2 texCoord;
    };

    struct VulkanMeshVertex
    {

        static std::array<VkVertexInputBindingDescription, 3> getVertexInputBindingDescription()
        {
            std::array<VkVertexInputBindingDescription, 3> binding_descriptions{};

            // position
            binding_descriptions[0].binding   = 0;
            binding_descriptions[0].stride    = sizeof(VulkanMeshVertexPostition);
            binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // normal
            binding_descriptions[1].binding   = 1;
            binding_descriptions[1].stride    = sizeof(VulkanMeshVertexNormal);
            binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // uv
            binding_descriptions[2].binding   = 2;
            binding_descriptions[2].stride    = sizeof(VulkanMeshVertexTexcoord);
            binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_descriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getVertexInputAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

            // position
            attribute_descriptions[0].binding  = 0; // 对应于在VkVertexInputBindingDescription中设置的绑定索引
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset   = offsetof(VulkanMeshVertexPostition, position);

            // varying blending
            attribute_descriptions[1].binding  = 1;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset   = offsetof(VulkanMeshVertexNormal, normal);

            attribute_descriptions[2].binding  = 1;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
            attribute_descriptions[2].offset   = offsetof(VulkanMeshVertexNormal, tangent);

            // varying texcoord
            attribute_descriptions[3].binding  = 2;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format   = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[3].offset   = offsetof(VulkanMeshVertexTexcoord, texCoord);

            return attribute_descriptions;
        }
    };


    class RenderMesh
    {
    public:
        std::string                            m_name;
        std::vector<VulkanMeshVertexPostition> m_positions;
        std::vector<VulkanMeshVertexNormal>    m_normals;
        std::vector<VulkanMeshVertexTexcoord>  m_texcoords;
        std::vector<uint16_t>                  m_indices;

//        Matrix4x4 model_matrix=Matrix4x4::getScale(0.4f,0.4f,0.4f);
        Matrix4x4 model_matrix=Matrix4x4::IDENTITY;
        uint16_t m_index_in_dynamic_buffer = 0;

        // TODO: 按层级加载，并赋上不同的材质
//        std::weak_ptr<RenderMesh> parent_mesh;
//        std::vector<std::shared_ptr<RenderMesh>> child_meshes;

        VkBuffer mesh_vertex_position_buffer = VK_NULL_HANDLE;
        VkBuffer mesh_vertex_normal_buffer   = VK_NULL_HANDLE;
        VkBuffer mesh_vertex_texcoord_buffer = VK_NULL_HANDLE;

        VkDeviceMemory mesh_vertex_position_buffer_memory = VK_NULL_HANDLE;
        VkDeviceMemory mesh_vertex_normal_buffer_memory   = VK_NULL_HANDLE;
        VkDeviceMemory mesh_vertex_texcoord_buffer_memory = VK_NULL_HANDLE;

        VkBuffer       mesh_index_buffer        = VK_NULL_HANDLE;
        VkDeviceMemory mesh_index_buffer_memory = VK_NULL_HANDLE;

        RenderMesh();

        ~RenderMesh();

        RenderMesh(RenderMesh const &) = delete;

        RenderMesh &operator=(RenderMesh const &) = delete;



        void ToGPU();

        void ReleaseFromDevice();
    };
}
#endif //XEXAMPLE_RENDER_MESH_H
