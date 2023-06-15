//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_UBO_H
#define XEXAMPLE_RENDER_UBO_H

#include <vulkan/vulkan.h>
#include "core/graphic/vulkan/vulkan_utils.h"
#include "core/math/math.h"
#include "render_common.h"

#define MAX_MODEL_COUNT 128*1024

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    // 保存场景中所有的模型的model_matrix、normal_matrix
    template<typename T>
    class RenderDynamicBuffer
    {
    public:
        VkDeviceSize           dynamic_alignment;
        VkDeviceSize           buffer_size;
        std::vector<T>         ubo_data_list;
        VkBuffer               dynamic_buffer;
        VkDeviceMemory         dynamic_buffer_memory;
        void                   *mapped_buffer_ptr;
        VkDescriptorBufferInfo buffer_info;

    public:
        RenderDynamicBuffer(VkDeviceSize buffer_size = -1)
        {
            // Calculate required alignment based on minimum device offset alignment
            VkDeviceSize min_ubo_alignment        = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            VkDeviceSize max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;

            max_uniform_buffer_range = std::min(max_uniform_buffer_range, buffer_size);

            dynamic_alignment = sizeof(T);
            if (min_ubo_alignment > 0)
            {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }
            buffer_size       = (max_uniform_buffer_range / dynamic_alignment) * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     dynamic_buffer, dynamic_buffer_memory);

            buffer_info.buffer = dynamic_buffer;
            // dynamic buffer的offset必须是dynamic_alignment的整数倍
            // 所谓dynamic buffer，就是可以在cpu端动态修改offset的buffer
            buffer_info.offset = 0;
            // dynamic buffer的range必须是dynamic_alignment，而非buffer_size
            buffer_info.range  = dynamic_alignment;
        }

        ~RenderDynamicBuffer()
        {
            vkDestroyBuffer(g_p_vulkan_context->_device, dynamic_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, nullptr);
        }

        RenderDynamicBuffer(const RenderDynamicBuffer &other) = delete;

        RenderDynamicBuffer &operator=(const RenderDynamicBuffer &other) = delete;

        void ToGPU()
        {
            uint32_t mapped_size = ubo_data_list.size() * dynamic_alignment;
            vkMapMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, 0,
                        mapped_size, 0, &mapped_buffer_ptr);

            // Aligned offset
            for (int i = 0; i < ubo_data_list.size(); ++i)
            {
                auto *data_ptr = (T *) ((uint64_t) mapped_buffer_ptr + (i * dynamic_alignment));
                *data_ptr = ubo_data_list[i];
            }

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = dynamic_buffer_memory;
            mappedMemoryRange.size   = mapped_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
            vkUnmapMemory(g_p_vulkan_context->_device, dynamic_buffer_memory);
        }
    };

    // 保存场景中全局信息，如相机矩阵，光照参数等
    class RenderPerFrameUBO
    {
    public:
        enum _preframe_buffer_data_block
        {
            _scene_info_block = 0,
            _light_info_block,
            _info_block_count
        };
        VkDeviceSize                         buffer_size;
        VulkanPerFrameSceneDefine            scene_data_ubo;
        VkDeviceSize                         per_frame_ubo_offset[_info_block_count];
        VulkanPerFrameDirectionalLightDefine directional_lights_ubo[MAX_DIRECTIONAL_LIGHT_COUNT];
        VkBuffer                             per_frame_buffer;
        VkDeviceMemory                       per_frame_buffer_memory;
        void                                 *mapped_buffer_ptr;
        std::vector<VkDescriptorBufferInfo>  buffer_infos;

        RenderPerFrameUBO()
        {
            // VUID-VkWriteDescriptorSet-descriptorType-00327
            // 描述符中的offset必须按照minUniformBufferOffsetAlignment对齐
            VkDeviceSize min_ubo_offset_align = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;

            per_frame_ubo_offset[_scene_info_block] = sizeof(VulkanPerFrameSceneDefine);
            if (min_ubo_offset_align > 0)
            {
                per_frame_ubo_offset[_scene_info_block] =
                        (per_frame_ubo_offset[_scene_info_block] + min_ubo_offset_align - 1) &
                        ~(min_ubo_offset_align - 1);
            }

            per_frame_ubo_offset[_light_info_block] =
                    sizeof(VulkanPerFrameDirectionalLightDefine) * MAX_DIRECTIONAL_LIGHT_COUNT;
            if (min_ubo_offset_align > 0)
            {
                per_frame_ubo_offset[_light_info_block] =
                        (per_frame_ubo_offset[_light_info_block] + min_ubo_offset_align - 1) &
                        ~(min_ubo_offset_align - 1);
            }

            buffer_size = per_frame_ubo_offset[_scene_info_block] + per_frame_ubo_offset[_light_info_block];

            VkDeviceSize no_coherent_atom_size = g_p_vulkan_context->_physical_device_properties.limits.nonCoherentAtomSize;
            buffer_size = (buffer_size + no_coherent_atom_size - 1) & ~(no_coherent_atom_size - 1);
//            dynamic_alignment = (dynamic_alignment + min_ubo_offset_align - 1) & ~(min_ubo_offset_align - 1);


            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     per_frame_buffer, per_frame_buffer_memory);

            vkMapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, 0,
                        buffer_size, 0, &mapped_buffer_ptr);

            buffer_infos.resize(_info_block_count);
            buffer_infos[_scene_info_block].buffer = per_frame_buffer;
            buffer_infos[_scene_info_block].offset = 0;
            buffer_infos[_scene_info_block].range  = sizeof(VulkanPerFrameSceneDefine);

            buffer_infos[_light_info_block].buffer = per_frame_buffer;
            buffer_infos[_light_info_block].offset = per_frame_ubo_offset[_scene_info_block];
            buffer_infos[_light_info_block].range  = sizeof(VulkanPerFrameDirectionalLightDefine);
        }


        ~RenderPerFrameUBO()
        {
            vkUnmapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory);
            vkDestroyBuffer(g_p_vulkan_context->_device, per_frame_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, nullptr);
        }

        RenderPerFrameUBO(const RenderPerFrameUBO &other) = delete;

        RenderPerFrameUBO &operator=(const RenderPerFrameUBO &other) = delete;

        void ToGPU()
        {
            assert(scene_data_ubo.directional_light_number > 0 &&
                   scene_data_ubo.directional_light_number <= MAX_DIRECTIONAL_LIGHT_COUNT);
            memcpy(mapped_buffer_ptr, &scene_data_ubo, sizeof(VulkanPerFrameSceneDefine));
            memcpy((void *) ((uint64_t) mapped_buffer_ptr + per_frame_ubo_offset[_scene_info_block]),
                   &directional_lights_ubo,
                   scene_data_ubo.directional_light_number * sizeof(VulkanPerFrameDirectionalLightDefine));

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = per_frame_buffer_memory;
            mappedMemoryRange.size   = buffer_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
        }
    };
}
#endif //XEXAMPLE_RENDER_UBO_H
