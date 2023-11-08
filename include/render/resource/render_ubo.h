//
// Created by kyros on 5/28/23.
//

#ifndef XEXAMPLE_RENDER_UBO_H
#define XEXAMPLE_RENDER_UBO_H

#include <vulkan/vulkan.h>
#include "core/graphic/vulkan/vulkan_utils.h"
#include "core/math/math.h"
#include "render_common.h"
#include <map>
#include <utility>

#define MAX_MODEL_COUNT 128*1024

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    class RenderReinterpretDynamicBuffer
    {
    public:
        uint32_t               data_size{};
        uint32_t               block_size{};
        VkDeviceSize           dynamic_alignment{};
        VkDeviceSize           buffer_size{};
        VkBuffer               dynamic_buffer{};
        VkDeviceMemory         dynamic_buffer_memory{};
        void                   *mapped_buffer_ptr{};
        VkDescriptorBufferInfo dynamic_info{};
        VkDescriptorBufferInfo static_info{};
    private:
        std::vector<uint8_t>                        ubo_data_list;
        VkDeviceSize                                min_ubo_alignment;
        VkDeviceSize                                max_uniform_buffer_range{};
        std::map<void *, std::function<void(void)>> update_callback_map;
    public:
        // buffer size = size * dynamic_alignment
        explicit RenderReinterpretDynamicBuffer(uint32_t size = 1, uint32_t block_size = 1)
        {
            // Calculate required alignment based on minimum device offset alignment
            min_ubo_alignment        = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;

            dynamic_alignment = block_size;
            if (min_ubo_alignment > 0)
            {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }

            if (size * dynamic_alignment > max_uniform_buffer_range)
                size = max_uniform_buffer_range / dynamic_alignment;

            data_size   = size;
            buffer_size = size * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     dynamic_buffer, dynamic_buffer_memory);

            ubo_data_list.resize(data_size * dynamic_alignment);

            dynamic_info.buffer = dynamic_buffer;
            // dynamic buffer的offset必须是dynamic_alignment的整数倍
            // 所谓dynamic buffer，就是可以在cpu端动态修改offset的buffer
            dynamic_info.offset = 0;
            // dynamic buffer的range必须是dynamic_alignment，而非buffer_size
            dynamic_info.range  = dynamic_alignment;
            // 当作static_buffer来用，要在shader中注意内存对齐的问题，通过合理插入__padding__来解决，通常按照float4对齐
            // 最好还是不要这么做，避免平台兼容性问题
            static_info.buffer  = dynamic_buffer;
            static_info.offset  = 0;
            static_info.range   = data_size * dynamic_alignment;
        }

        ~RenderReinterpretDynamicBuffer()
        {
            vkDestroyBuffer(g_p_vulkan_context->_device, dynamic_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, nullptr);
        }

        RenderReinterpretDynamicBuffer(RenderReinterpretDynamicBuffer &&other)
        {
            dynamic_alignment     = other.dynamic_alignment;
            buffer_size           = other.buffer_size;
            ubo_data_list         = std::move(other.ubo_data_list);
            dynamic_buffer        = other.dynamic_buffer;
            dynamic_buffer_memory = other.dynamic_buffer_memory;
            mapped_buffer_ptr     = other.mapped_buffer_ptr;
            dynamic_info          = other.dynamic_info;
            static_info           = other.static_info;

            other.dynamic_buffer        = VK_NULL_HANDLE;
            other.dynamic_buffer_memory = VK_NULL_HANDLE;
            other.mapped_buffer_ptr     = nullptr;
            other.dynamic_info.buffer   = VK_NULL_HANDLE;
            other.static_info.buffer    = VK_NULL_HANDLE;
        }


        RenderReinterpretDynamicBuffer(const RenderReinterpretDynamicBuffer &other) = delete;

        RenderReinterpretDynamicBuffer &operator=(const RenderReinterpretDynamicBuffer &other) = delete;

        void resize(uint32_t _size, uint32_t _block_size)
        {
            if (_size <= 0 || (_size == data_size && block_size == _block_size))
                return;

            if (dynamic_buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(g_p_vulkan_context->_device, dynamic_buffer, nullptr);
            }

            if (dynamic_buffer_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, nullptr);
            }

            data_size         = _size;
            block_size        = _block_size;
            dynamic_alignment = block_size;

            if (min_ubo_alignment > 0)
            {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }

            if (data_size * dynamic_alignment > max_uniform_buffer_range)
                data_size = max_uniform_buffer_range / dynamic_alignment;

            buffer_size = data_size * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     dynamic_buffer, dynamic_buffer_memory);

            ubo_data_list.resize(data_size * dynamic_alignment);
            dynamic_info.buffer = dynamic_buffer;
            dynamic_info.offset = 0;
            dynamic_info.range  = dynamic_alignment;

            static_info.buffer = dynamic_buffer;
            static_info.offset = 0;
            static_info.range  = data_size * dynamic_alignment;

            for (auto &pair: update_callback_map)
            {
                pair.second();
            }
        }

        void DescribeUpdate(void *key, std::function<void(void)> update_callback)
        {
            update_callback_map[key] = std::move(update_callback);
        }

        // index是第几个block，size是block的大小
        void SetData(uint32_t index, void *data, uint32_t size)
        {
            assert(index < data_size);
            assert(size <= dynamic_alignment);
            memcpy(ubo_data_list.data() + index * dynamic_alignment, data, size);
        }

        void ToGPU()
        {
            uint32_t mapped_size = ubo_data_list.size();
            vkMapMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, 0,
                        mapped_size, 0, &mapped_buffer_ptr);

            memcpy(mapped_buffer_ptr, ubo_data_list.data(), mapped_size);

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = dynamic_buffer_memory;
            mappedMemoryRange.size   = mapped_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
            vkUnmapMemory(g_p_vulkan_context->_device, dynamic_buffer_memory);
        }
    };

    // 保存场景中所有的模型的model_matrix、normal_matrix
    template<typename T>
    class RenderDynamicBuffer
    {
    public:
        VkDeviceSize           dynamic_alignment{};
        VkDeviceSize           buffer_size{};
        std::vector<T>         ubo_data_list;
        VkBuffer               dynamic_buffer{};
        VkDeviceMemory         dynamic_buffer_memory{};
        void                   *mapped_buffer_ptr{};
        VkDescriptorBufferInfo dynamic_info{};
        VkDescriptorBufferInfo static_info{};
    private:
        VkDeviceSize                                max_uniform_buffer_range;
        std::map<void *, std::function<void(void)>> update_callback_map;
    public:
        explicit RenderDynamicBuffer(VkDeviceSize size = -1)
        {
            // Calculate required alignment based on minimum device offset alignment
            VkDeviceSize min_ubo_alignment = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;

            dynamic_alignment = sizeof(T);
            if (min_ubo_alignment > 0)
            {
                dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
            }

            if (size * dynamic_alignment > max_uniform_buffer_range)
                size = max_uniform_buffer_range / dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     size * dynamic_alignment,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     dynamic_buffer, dynamic_buffer_memory);

            dynamic_info.buffer = dynamic_buffer;
            // dynamic buffer的offset必须是dynamic_alignment的整数倍
            // 所谓dynamic buffer，就是可以在cpu端动态修改offset的buffer
            dynamic_info.offset = 0;
            // dynamic buffer的range必须是dynamic_alignment，而非buffer_size
            dynamic_info.range  = dynamic_alignment;
            // 当作static_buffer来用，要在shader中注意内存对齐的问题，通过合理插入__padding__来解决，通常按照float4对齐
            // 最好还是不要这么做，避免平台兼容性问题
            static_info.buffer  = dynamic_buffer;
            static_info.offset  = 0;
            static_info.range   = size * dynamic_alignment;
        }

        ~RenderDynamicBuffer()
        {
            vkDestroyBuffer(g_p_vulkan_context->_device, dynamic_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, nullptr);
        }

        RenderDynamicBuffer(RenderDynamicBuffer &&other)
        {
            dynamic_alignment     = other.dynamic_alignment;
            buffer_size           = other.buffer_size;
            ubo_data_list         = std::move(other.ubo_data_list);
            dynamic_buffer        = other.dynamic_buffer;
            dynamic_buffer_memory = other.dynamic_buffer_memory;
            mapped_buffer_ptr     = other.mapped_buffer_ptr;
            dynamic_info          = other.dynamic_info;
            static_info           = other.static_info;

            other.dynamic_buffer        = VK_NULL_HANDLE;
            other.dynamic_buffer_memory = VK_NULL_HANDLE;
            other.mapped_buffer_ptr     = nullptr;
            other.dynamic_info.buffer   = VK_NULL_HANDLE;
            other.static_info.buffer    = VK_NULL_HANDLE;
        }


        RenderDynamicBuffer(const RenderDynamicBuffer &other) = delete;

        RenderDynamicBuffer &operator=(const RenderDynamicBuffer &other) = delete;

        void resize(uint32_t size)
        {
            if (size <= 0 || size == ubo_data_list.size())
                return;

            if (dynamic_buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(g_p_vulkan_context->_device, dynamic_buffer, nullptr);
            }

            if (dynamic_buffer_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(g_p_vulkan_context->_device, dynamic_buffer_memory, nullptr);
            }

            ubo_data_list.resize(size);
            buffer_size = ubo_data_list.size() * dynamic_alignment;

            assert(buffer_size <= max_uniform_buffer_range);

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     dynamic_buffer, dynamic_buffer_memory);

            dynamic_info.buffer = dynamic_buffer;
            dynamic_info.offset = 0;
            dynamic_info.range  = dynamic_alignment;

            static_info.buffer = dynamic_buffer;
            static_info.offset = 0;
            static_info.range  = size * dynamic_alignment;

            for (auto &pair: update_callback_map)
            {
                pair.second();
            }
        }

        void DescribeUpdate(void *key, std::function<void(void)> update_callback)
        {
            update_callback_map[key] = std::move(update_callback);
        }

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

    template<typename T>
    class RenderStaticBuffer
    {
    public:
        VkDeviceSize           block_size;
        VkDeviceSize           buffer_size;
        std::vector<T>         ubo_data_list;
        VkBuffer               static_buffer;
        VkDeviceMemory         static_buffer_memory;
        void                   *mapped_buffer_ptr;
        VkDescriptorBufferInfo static_info;
    private:
        VkDeviceSize                                max_uniform_buffer_range;
        std::map<void *, std::function<void(void)>> update_callback_map;

    public:
        RenderStaticBuffer(VkDeviceSize size = 1)
        {
            // Calculate required alignment based on minimum device offset alignment
            max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;
            block_size               = sizeof(T);

            static_buffer        = VK_NULL_HANDLE;
            static_buffer_memory = VK_NULL_HANDLE;

            resize(size);
        }

        ~RenderStaticBuffer()
        {
            vkDestroyBuffer(g_p_vulkan_context->_device, static_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, static_buffer_memory, nullptr);
            static_buffer        = VK_NULL_HANDLE;
            static_buffer_memory = VK_NULL_HANDLE;
        }

        RenderStaticBuffer(const RenderStaticBuffer &other) = delete;

        RenderStaticBuffer &operator=(const RenderStaticBuffer &other) = delete;

        void resize(uint32_t size)
        {
            if (size <= 0 || size == ubo_data_list.size())
                return;

            if (static_buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(g_p_vulkan_context->_device, static_buffer, nullptr);
            }

            if (static_buffer_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(g_p_vulkan_context->_device, static_buffer_memory, nullptr);
            }

            ubo_data_list.resize(size);
            buffer_size = ubo_data_list.size() * block_size;

            assert(buffer_size <= max_uniform_buffer_range);

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     static_buffer, static_buffer_memory);

            static_info.buffer = static_buffer;
            static_info.offset = 0;
            static_info.range  = buffer_size;

            for (auto &pair: update_callback_map)
            {
                pair.second();
            }
        }

        void DescribeUpdate(void *key, std::function<void(void)> update_callback)
        {
            update_callback_map[key] = std::move(update_callback);
        }

        void ToGPU()
        {
            uint32_t mapped_size = ubo_data_list.size() * block_size;
            vkMapMemory(g_p_vulkan_context->_device, static_buffer_memory, 0,
                        mapped_size, 0, &mapped_buffer_ptr);

            // Aligned offset
            for (int i = 0; i < ubo_data_list.size(); ++i)
            {
                auto *data_ptr = (T *) ((uint64_t) mapped_buffer_ptr + (i * block_size));
                *data_ptr = ubo_data_list[i];
            }

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = static_buffer_memory;
            mappedMemoryRange.size   = mapped_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
            vkUnmapMemory(g_p_vulkan_context->_device, static_buffer_memory);
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
        uint32_t                            data_size{};
        uint32_t                            block_size{};
        VkDeviceSize                        buffer_size;
        VkDeviceSize                        dynamic_alignment{};
        VkDeviceSize                        per_frame_ubo_range[_info_block_count]{};
        VkDeviceSize                        per_frame_ubo_offset[_info_block_count]{};
        VkBuffer                            per_frame_buffer{};
        VkDeviceMemory                      per_frame_buffer_memory{};
        void                                *mapped_buffer_ptr{};
        std::vector<VkDescriptorBufferInfo> buffer_infos;
    private:
        std::vector<uint8_t> ubo_data_list;
        VkDeviceSize         min_ubo_alignment;
        VkDeviceSize         max_uniform_buffer_range{};

    public:
        RenderPerFrameUBO()
        {
            // VUID-VkWriteDescriptorSet-descriptorType-00327
            // 描述符中的offset必须按照minUniformBufferOffsetAlignment对齐
            min_ubo_alignment        = g_p_vulkan_context->_physical_device_properties.limits.minUniformBufferOffsetAlignment;
            max_uniform_buffer_range = g_p_vulkan_context->_physical_device_properties.limits.maxUniformBufferRange;

            data_size = 3;

            per_frame_ubo_range[_scene_info_block] = sizeof(VulkanPerFrameSceneDefine);
            if (min_ubo_alignment > 0)
            {
                per_frame_ubo_range[_scene_info_block] =
                        (per_frame_ubo_range[_scene_info_block] + min_ubo_alignment - 1) &
                        ~(min_ubo_alignment - 1);
            }

            per_frame_ubo_range[_light_info_block] =
                    sizeof(VulkanPerFrameDirectionalLightDefine) * MAX_DIRECTIONAL_LIGHT_COUNT;
            if (min_ubo_alignment > 0)
            {
                per_frame_ubo_range[_light_info_block] =
                        (per_frame_ubo_range[_light_info_block] + min_ubo_alignment - 1) &
                        ~(min_ubo_alignment - 1);
            }

            per_frame_ubo_offset[_scene_info_block] = 0;
            per_frame_ubo_offset[_light_info_block] = per_frame_ubo_range[_scene_info_block];
            block_size = per_frame_ubo_range[_scene_info_block] + per_frame_ubo_range[_light_info_block];
            dynamic_alignment = block_size;

            buffer_size = dynamic_alignment * data_size;

//            VkDeviceSize no_coherent_atom_size = g_p_vulkan_context->_physical_device_properties.limits.nonCoherentAtomSize;
//            buffer_size = (buffer_size + no_coherent_atom_size - 1) & ~(no_coherent_atom_size - 1);
//            block = (block + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);


            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     per_frame_buffer, per_frame_buffer_memory);

            ubo_data_list.resize(buffer_size);

            vkMapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, 0,
                        buffer_size, 0, &mapped_buffer_ptr);

            buffer_infos.resize(_info_block_count);
            buffer_infos[_scene_info_block].buffer = per_frame_buffer;
            buffer_infos[_scene_info_block].offset = per_frame_ubo_offset[_scene_info_block];
            buffer_infos[_scene_info_block].range  = sizeof(VulkanPerFrameSceneDefine);

            buffer_infos[_light_info_block].buffer = per_frame_buffer;
            buffer_infos[_light_info_block].offset = per_frame_ubo_offset[_light_info_block];
            buffer_infos[_light_info_block].range  =
                    sizeof(VulkanPerFrameDirectionalLightDefine) * MAX_DIRECTIONAL_LIGHT_COUNT;
        }

        ~RenderPerFrameUBO()
        {
            vkUnmapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory);
            vkDestroyBuffer(g_p_vulkan_context->_device, per_frame_buffer, nullptr);
            vkFreeMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, nullptr);
        }

        RenderPerFrameUBO(const RenderPerFrameUBO &other) = default;

        RenderPerFrameUBO &operator=(const RenderPerFrameUBO &other) = delete;

        void resize(uint32_t size)
        {
            if (size <= 0 || size == data_size)
                return;

            if (per_frame_buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(g_p_vulkan_context->_device, per_frame_buffer, nullptr);
            }

            if (per_frame_buffer_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, nullptr);
            }

            data_size   = size;
            buffer_size = data_size * dynamic_alignment;

            VulkanUtil::createBuffer(g_p_vulkan_context,
                                     buffer_size,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     per_frame_buffer, per_frame_buffer_memory);

            ubo_data_list.resize(buffer_size);

            vkMapMemory(g_p_vulkan_context->_device, per_frame_buffer_memory, 0,
                        buffer_size, 0, &mapped_buffer_ptr);

        }

        void SetData(uint32_t data_index, void *data, uint32_t size, _preframe_buffer_data_block block)
        {
            assert(data_index < data_size);
            assert(size <= per_frame_ubo_range[block]);
            memcpy(ubo_data_list.data() + data_index * dynamic_alignment + per_frame_ubo_offset[block], data, size);
        }

        void ToGPU()
        {
            uint32_t mapped_size = ubo_data_list.size();
            memcpy(mapped_buffer_ptr, ubo_data_list.data(), mapped_size);

            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.memory = per_frame_buffer_memory;
            mappedMemoryRange.size   = buffer_size;

            vkFlushMappedMemoryRanges(g_p_vulkan_context->_device, 1, &mappedMemoryRange);
        }
    };
}
#endif //XEXAMPLE_RENDER_UBO_H
