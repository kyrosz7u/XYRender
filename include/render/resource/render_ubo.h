//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_UBO_H
#define XEXAMPLE_RENDER_UBO_H

#include <vulkan/vulkan.h>
#include "core/graphic/vulkan/vulkan_utils.h"
#include "core/math/math.h"

#define MAX_MODEL_COUNT 128*1024
#define MAX_DIRECTIONAL_LIGHT_COUNT 16

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    struct VulkanModelDefine
    {
        Math::Matrix4x4 model;
    };
    struct VulkanPerFrameSceneDefine
    {
        Math::Matrix4x4 proj_view;
        Math::Vector3   camera_pos;
        uint32_t        directional_light_number;
    };
    struct VulkanPerFrameDirectionalLightDefine
    {
        float         intensity;
        Math::Vector3 direction;
        Math::Color   color;
    };


    class RenderModelUBOList
    {
    public:
        VkDeviceSize                   dynamic_alignment;
        VkDeviceSize                   buffer_size;
        std::vector<VulkanModelDefine> ubo_data_list;
        VkBuffer                       model_ubo_dynamic_buffer;
        VkDeviceMemory                 model_ubo_dynamic_buffer_memory;
        void                           *mapped_buffer_ptr;
        VkDescriptorBufferInfo         buffer_descriptor;

    public:
        RenderModelUBOList()
        {
            // Calculate required alignment based on minimum device offset alignment
            VkDeviceSize min_ubo_alignment        = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            VkDeviceSize max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;

            dynamic_alignment = sizeof(VulkanModelDefine);
            if (min_ubo_alignment > 0)
            {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }
            buffer_size       = (max_uniform_buffer_range / dynamic_alignment) * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     model_ubo_dynamic_buffer, model_ubo_dynamic_buffer_memory);

            buffer_descriptor.buffer = model_ubo_dynamic_buffer;
            buffer_descriptor.offset = 0;
            // dynamic buffer的range必须是dynamic_alignment，而非buffer_size
            buffer_descriptor.range  = dynamic_alignment;
        }

        ~RenderModelUBOList()
        {
            vkUnmapMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory);
            vkDestroyBuffer(g_p_vulkan_context->_device, model_ubo_dynamic_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory, nullptr);
        }

        RenderModelUBOList(const RenderModelUBOList &other) = delete;

        RenderModelUBOList &operator=(const RenderModelUBOList &other) = delete;
//        RenderModelUBOList(RenderModelUBOList&& other) noexcept
//        {
//            ubo_data_list = std::move(other.ubo_data_list);
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
            uint32_t mapped_size = ubo_data_list.size() * dynamic_alignment;
            vkMapMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory, 0,
                        mapped_size, 0, &mapped_buffer_ptr);

            // Aligned offset
            for (int i = 0; i < ubo_data_list.size(); ++i)
            {
                Math::Matrix4x4 *model_matrix = (Math::Matrix4x4 *) ((uint64_t) mapped_buffer_ptr +
                                                                     (i * dynamic_alignment));
                *model_matrix = ubo_data_list[i].model;
            }

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = model_ubo_dynamic_buffer_memory;
            mappedMemoryRange.size   = mapped_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
            vkUnmapMemory(g_p_vulkan_context->_device, model_ubo_dynamic_buffer_memory);
        }
    };

    class RenderPerFrameUBO
    {
    public:
        enum _preframe_buffer_data_block
        {
            _scene_block = 0,
            _light_block,
            _block_count
        };
        VkDeviceSize                         buffer_size;
        VulkanPerFrameSceneDefine            per_frame_ubo;
        VulkanPerFrameDirectionalLightDefine directional_light_list_ubo[MAX_DIRECTIONAL_LIGHT_COUNT];
        VkBuffer                             per_frame_buffer;
        VkDeviceMemory                       per_frame_buffer_memory;
        void                                 *mapped_buffer_ptr;
        std::vector<VkDescriptorBufferInfo>  buffer_descriptors;

        RenderPerFrameUBO()
        {
            buffer_size                        =
                    sizeof(VulkanPerFrameSceneDefine) +
                    sizeof(VulkanPerFrameDirectionalLightDefine) * MAX_DIRECTIONAL_LIGHT_COUNT;
            VkDeviceSize no_coherent_atom_size = g_p_vulkan_context->_physical_device_properties.limits.nonCoherentAtomSize;
            buffer_size = (buffer_size + no_coherent_atom_size - 1) & ~(no_coherent_atom_size - 1);
//            dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);


            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     per_frame_buffer, per_frame_buffer_memory);

            vkMapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, 0,
                        buffer_size, 0, &mapped_buffer_ptr);

            buffer_descriptors.resize(_block_count);
            buffer_descriptors[_scene_block].buffer = per_frame_buffer;
            buffer_descriptors[_scene_block].offset = 0;
            buffer_descriptors[_scene_block].range  = sizeof(VulkanPerFrameSceneDefine);

            buffer_descriptors[_light_block].buffer = per_frame_buffer;
            buffer_descriptors[_light_block].offset = sizeof(VulkanPerFrameSceneDefine);
            buffer_descriptors[_light_block].range  = sizeof(VulkanPerFrameDirectionalLightDefine);
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
            assert(per_frame_ubo.directional_light_number > 0 &&
                   per_frame_ubo.directional_light_number <= MAX_DIRECTIONAL_LIGHT_COUNT);
            memcpy(mapped_buffer_ptr, &per_frame_ubo, sizeof(VulkanPerFrameSceneDefine));
            memcpy((void *) ((uint64_t) mapped_buffer_ptr + sizeof(VulkanPerFrameSceneDefine)),
                   &directional_light_list_ubo,
                   per_frame_ubo.directional_light_number * sizeof(VulkanPerFrameDirectionalLightDefine));

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = per_frame_buffer_memory;
            mappedMemoryRange.size   = buffer_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
        }
    };
}
#endif //XEXAMPLE_RENDER_UBO_H
