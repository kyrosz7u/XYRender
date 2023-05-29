//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_UBO_H
#define XEXAMPLE_RENDER_UBO_H

#include <vulkan/vulkan.h>
#include "graphic/vulkan/vulkan_utils.h"
#include "math/Math.h"

#define MAX_MODEL_COUNT 128*1024

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    struct VulkanModelDefine
    {
        Math::Matrix4x4 model;
    };
    struct VulkanPerFrameDefine
    {
        Math::Matrix4x4 viewProj;
        Math::Vector3   cameraPos;
        Math::Vector3   dirLightDir;
    };

    class RenderModelUBOList
    {
    public:
        VkDeviceSize dynamic_alignment;
        VkDeviceSize buffer_size;
        std::vector<VulkanModelDefine> model_ubo_list;
        VkBuffer  model_ubo_dynamic_buffer;
        VkDeviceMemory model_ubo_dynamic_buffer_memory;
        void* mapped_buffer_ptr;
        VkDescriptorBufferInfo buffer_descriptor;

    public:
        RenderModelUBOList()
        {
            // Calculate required alignment based on minimum device offset alignment
            size_t min_ubo_alignment = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            dynamic_alignment = sizeof(VulkanModelDefine);
            if (min_ubo_alignment > 0) {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }
            buffer_size = MAX_MODEL_COUNT * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     model_ubo_dynamic_buffer, model_ubo_dynamic_buffer_memory);

            buffer_descriptor.buffer = model_ubo_dynamic_buffer;
            buffer_descriptor.offset = 0;
            buffer_descriptor.range = buffer_size;
        }

        ~RenderModelUBOList()
        {
            vkUnmapMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory);
            vkDestroyBuffer(g_p_vulkan_context->_device, model_ubo_dynamic_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory, nullptr);
        }

        RenderModelUBOList(const RenderModelUBOList& other) =delete;
        RenderModelUBOList& operator=(const RenderModelUBOList& other) =delete;
//        RenderModelUBOList(RenderModelUBOList&& other) noexcept
//        {
//            model_ubo_list = std::move(other.model_ubo_list);
//            model_ubo_dynamic_buffer = std::move(other.model_ubo_dynamic_buffer);
//        }

//        void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
//        {
//            buffer_descriptor.buffer = model_ubo_dynamic_buffer;
//            buffer_descriptor.offset = offset;
//            buffer_descriptor.range = size;
//        }

        void ToGPU()
        {
            uint32_t mapped_size = model_ubo_list.size() * dynamic_alignment;
            vkMapMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory, 0,
                        mapped_size, 0, &mapped_buffer_ptr);

            // Aligned offset
            for (int i = 0; i < model_ubo_list.size(); ++i)
            {
                Math::Matrix4x4* model_matrix = (Math::Matrix4x4*)((uint64_t)mapped_buffer_ptr + (i * dynamic_alignment));
                *model_matrix= model_ubo_list[i].model;
            }

            VkMappedMemoryRange mappedMemoryRange {};
            mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = model_ubo_dynamic_buffer_memory;
            mappedMemoryRange.size = mapped_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
        }
    };

    class RenderPerFrameUBO
    {
    public:
        VkDeviceSize buffer_size;
        VulkanPerFrameDefine per_frame_ubo_list;
        VkBuffer  per_frame_buffer;
        VkDeviceMemory per_frame_buffer_memory;
        void* mapped_buffer_ptr;
        VkDescriptorBufferInfo buffer_descriptor;

        RenderPerFrameUBO()
        {
            buffer_size = sizeof(VulkanPerFrameDefine);
            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     per_frame_buffer, per_frame_buffer_memory);

            vkMapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, 0,
                        sizeof(VulkanModelDefine) * MAX_MODEL_COUNT, 0, &mapped_buffer_ptr);

            buffer_descriptor.buffer = per_frame_buffer;
            buffer_descriptor.offset = 0;
            buffer_descriptor.range = buffer_size;
        }

        ~RenderPerFrameUBO()
        {
            vkUnmapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory);
            vkDestroyBuffer(g_p_vulkan_context->_device, per_frame_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, nullptr);
        }

        RenderPerFrameUBO(const RenderPerFrameUBO& other) =delete;
        RenderPerFrameUBO& operator=(const RenderPerFrameUBO& other) =delete;

        void ToGPU()
        {
            memcpy(mapped_buffer_ptr, &per_frame_ubo_list, sizeof(VulkanPerFrameDefine));

            VkMappedMemoryRange mappedMemoryRange {};
            mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = per_frame_buffer_memory;
            mappedMemoryRange.size = sizeof(VulkanPerFrameDefine);

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
        }
    };
}
#endif //XEXAMPLE_RENDER_UBO_H
