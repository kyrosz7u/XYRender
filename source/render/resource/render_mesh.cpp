//
// Created by kyros on 5/27/23.
//

#include "render/resource/render_mesh.h"
#include "graphic/vulkan/vulkan_utils.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace VulkanAPI;
using namespace RenderSystem;

RenderMesh::RenderMesh()
{
    assert(g_p_vulkan_context);
}

RenderMesh::~RenderMesh()
{
    if (mesh_vertex_position_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_position_buffer, nullptr);
    }
    if (mesh_vertex_position_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_position_buffer_memory, nullptr);
    }
    if (mesh_vertex_normal_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_normal_buffer, nullptr);
    }
    if (mesh_vertex_normal_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_normal_buffer_memory, nullptr);
    }
    if (mesh_vertex_texcoord_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_texcoord_buffer, nullptr);
    }
    if (mesh_vertex_texcoord_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_texcoord_buffer_memory, nullptr);
    }
    if (mesh_index_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_index_buffer, nullptr);
    }
    if (mesh_index_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_index_buffer_memory, nullptr);
    }
}

void RenderMesh::ToGPU()
{
    VkDeviceSize vertex_position_buffer_size = sizeof(VulkanMeshVertexPostition) * m_positions.size();
    VkDeviceSize vertex_normal_buffer_size   = sizeof(VulkanMeshVertexNormal) * m_normals.size();
    VkDeviceSize vertex_texcoord_buffer_size = sizeof(VulkanMeshVertexTexcoord) * m_texcoords.size();
    VkDeviceSize index_buffer_size           = sizeof(uint16_t) * m_indices.size();

    VkDeviceSize staging_buffer_size = vertex_position_buffer_size + vertex_normal_buffer_size +
                                       vertex_texcoord_buffer_size + index_buffer_size;

    VkDeviceSize vertex_position_offset = 0;
    VkDeviceSize vertex_normal_offset   = vertex_position_offset + vertex_position_buffer_size;
    VkDeviceSize vertex_texcoord_offset = vertex_normal_offset + vertex_normal_buffer_size;
    VkDeviceSize index_offset           = vertex_texcoord_offset + vertex_texcoord_buffer_size;


    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingMemory;

    VulkanUtil::createBuffer(g_p_vulkan_context,
                             staging_buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer, stagingMemory);


    void *data;
    vkMapMemory(g_p_vulkan_context->_device, stagingMemory, 0, staging_buffer_size, 0, &data);
    uint8_t *buffer_ptr = (uint8_t *) data;
    memcpy(buffer_ptr + vertex_position_offset, m_positions.data(), vertex_position_buffer_size);
    memcpy(buffer_ptr + vertex_normal_offset, m_normals.data(), vertex_normal_buffer_size);
    memcpy(buffer_ptr + vertex_texcoord_offset, m_texcoords.data(), vertex_texcoord_buffer_size);
    memcpy(buffer_ptr + index_offset, m_indices.data(), index_buffer_size);
    vkUnmapMemory(g_p_vulkan_context->_device, stagingMemory);

    VulkanUtil::createBuffer(g_p_vulkan_context,
                             vertex_position_buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             mesh_vertex_position_buffer, mesh_vertex_position_buffer_memory);

    VulkanUtil::createBuffer(g_p_vulkan_context,
                             vertex_normal_buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             mesh_vertex_normal_buffer, mesh_vertex_normal_buffer_memory);

    VulkanUtil::createBuffer(g_p_vulkan_context,
                             vertex_texcoord_buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             mesh_vertex_texcoord_buffer, mesh_vertex_texcoord_buffer_memory);

    VulkanUtil::createBuffer(g_p_vulkan_context,
                             index_buffer_size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             mesh_index_buffer, mesh_index_buffer_memory);

    VulkanUtil::copyBuffer(g_p_vulkan_context, stagingBuffer, mesh_vertex_position_buffer,
                           vertex_position_offset,0, vertex_position_buffer_size);
    VulkanUtil::copyBuffer(g_p_vulkan_context, stagingBuffer, mesh_vertex_normal_buffer,
                             vertex_normal_offset,0, vertex_normal_buffer_size);
    VulkanUtil::copyBuffer(g_p_vulkan_context, stagingBuffer, mesh_vertex_texcoord_buffer,
                           vertex_texcoord_offset,0, vertex_texcoord_buffer_size);
    VulkanUtil::copyBuffer(g_p_vulkan_context, stagingBuffer, mesh_index_buffer,
                             index_offset,0, index_buffer_size);

    vkDestroyBuffer(g_p_vulkan_context->_device, stagingBuffer, nullptr);
    vkFreeMemory(g_p_vulkan_context->_device, stagingMemory, nullptr);
}


void RenderMesh::ReleaseFromDevice()
{
    if (mesh_vertex_position_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_position_buffer, nullptr);
        mesh_vertex_position_buffer = VK_NULL_HANDLE;
    }
    if (mesh_vertex_position_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_position_buffer_memory, nullptr);
        mesh_vertex_position_buffer_memory = VK_NULL_HANDLE;
    }
    if (mesh_vertex_normal_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_normal_buffer, nullptr);
        mesh_vertex_normal_buffer = VK_NULL_HANDLE;
    }
    if (mesh_vertex_normal_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_normal_buffer_memory, nullptr);
        mesh_vertex_normal_buffer_memory = VK_NULL_HANDLE;
    }
    if (mesh_vertex_texcoord_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_vertex_texcoord_buffer, nullptr);
        mesh_vertex_texcoord_buffer = VK_NULL_HANDLE;
    }
    if (mesh_vertex_texcoord_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_vertex_texcoord_buffer_memory, nullptr);
        mesh_vertex_texcoord_buffer_memory = VK_NULL_HANDLE;
    }
    if (mesh_index_buffer)
    {
        vkDestroyBuffer(g_p_vulkan_context->_device, mesh_index_buffer, nullptr);
        mesh_index_buffer = VK_NULL_HANDLE;
    }
    if (mesh_index_buffer_memory)
    {
        vkFreeMemory(g_p_vulkan_context->_device, mesh_index_buffer_memory, nullptr);
        mesh_index_buffer_memory = VK_NULL_HANDLE;
    }
}




