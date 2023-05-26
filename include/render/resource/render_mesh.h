//
// Created by kyros on 5/25/23.
//

#include <vulkan/vulkan.h>
#include "math/Math.h"

#include <array>

using namespace Math;

#ifndef XEXAMPLE_RENDER_MESH_H
#define XEXAMPLE_RENDER_MESH_H

struct VulkanMeshVertexPostition
{
    Vector3 position;
};

struct VulkanMeshVertexNormal
{
    Vector3 normal;
    Vector3 tangent;
};

struct VulkanMeshVertexTexcoord
{
    Vector2 texcoord;
};

struct MeshVertex
{

    static std::array<VkVertexInputBindingDescription, 3> getVertexInputBindingDescription()
    {
        std::array<VkVertexInputBindingDescription, 3> binding_descriptions {};

        // position
        binding_descriptions[0].binding   = 0;
        binding_descriptions[0].stride    = sizeof(VulkanMeshVertexPostition);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        // varying normal
        binding_descriptions[1].binding   = 1;
        binding_descriptions[1].stride    = sizeof(VulkanMeshVertexNormal);
        binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        // varying uv
        binding_descriptions[2].binding   = 2;
        binding_descriptions[2].stride    = sizeof(VulkanMeshVertexTexcoord);
        binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_descriptions;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getVertexInputAttributeDescription()
    {
        std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions {};

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
        attribute_descriptions[3].offset   = offsetof(VulkanMeshVertexTexcoord, texcoord);

        return attribute_descriptions;
    }
};


class VulkanMesh
{
public:
    uint32_t mesh_vertex_count;
    uint32_t mesh_index_count;
    
    RHIBuffer*    mesh_vertex_position_buffer;

    RHIBuffer*    mesh_vertex_normal_buffer;

    RHIBuffer*    mesh_vertex_texcoord_buffer;

    RHIBuffer*    mesh_index_buffer;
};

#endif //XEXAMPLE_RENDER_MESH_H
