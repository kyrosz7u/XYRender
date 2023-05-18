//
// Created by kyrosz7u on 4/7/23.
//
#include "graphic/vulkan/vulkan_manager.h"

using namespace VulkanAPI;

#ifndef NDEBUG
bool VulkanManager::m_enable_validation_Layers = true;
bool VulkanManager::m_enable_debug_utils_label = true;
#else
bool VulkanManager::m_enable_validation_Layers  = false;
bool VulkanManager::m_enable_debug_utils_label  = false;
#endif

int VulkanManager::initialize(std::shared_ptr<VulkanContext> _context)
{
    m_vulkan_context = _context;
    
//    m_global_render_resource.initialize(m_vulkan_context, m_max_frames_in_flight);
//
//    PRenderPassBase::m_render_config._enable_debug_untils_label = m_enable_debug_utils_label;
//    PRenderPassBase::m_render_config._enable_point_light_shadow = m_enable_point_light_shadow;
//    PRenderPassBase::m_render_config._enable_validation_Layers  = m_enable_validation_Layers;
//
//    PRenderPassBase::m_command_info._viewport = m_viewport;
//    PRenderPassBase::m_command_info._scissor  = m_scissor;
//
//    // global textures for IBL
//    PIBLResourceData ibl_resource_data = m_global_render_resource.getIBLTextureData(&scene, pilot_renderer);
//    updateGlobalTexturesForIBL(ibl_resource_data);
//
//    // global textures for color grading
//    PColorGradingResourceData color_grading_resource_data =
//            m_global_render_resource.getColorGradingTextureData(&scene, pilot_renderer);
//    updateGlobalTexturesForColorGrading(color_grading_resource_data);
//

//    if (initializeCommandPool() && initializeDescriptorPool() && createSyncPrimitives() && initializeCommandBuffers() &&
//        initializeRenderPass())
//        return 1;
//    else
//        return 0;

    return 0;
}

// command pool for submitting drawing commands
bool VulkanManager::initializeCommandPool()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext            = NULL;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    command_pool_create_info.queueFamilyIndex = m_vulkan_context->_queue_indices.graphicsFamily.value();

    for (uint32_t i = 0; i < m_max_frames_in_flight; ++i)
    {
        if (vkCreateCommandPool(m_vulkan_context->_device, &command_pool_create_info, NULL, &m_command_pools[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk create command pool");
        }
    }
    return true;
}

// allocate command buffer for drawing commands
bool VulkanManager::initializeCommandBuffers()
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info {};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;

    for (uint32_t i = 0; i < m_max_frames_in_flight; ++i)
    {
        command_buffer_allocate_info.commandPool = m_command_pools[i];

        if (vkAllocateCommandBuffers(m_vulkan_context->_device, &command_buffer_allocate_info, &m_command_buffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk allocate command buffers");
        }
    }
    return true;
}

bool VulkanManager::initializeDescriptorPool()
{
    // Since DescriptorSet should be treated as asset in Vulkan, DescriptorPool
    // should be big enough, and thus we can sub-allocate DescriptorSet from
    // DescriptorPool merely as we sub-allocate Buffer/Image from DeviceMemory.

    VkDescriptorPoolSize pool_sizes[6];
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    pool_sizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;
    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[1].descriptorCount = 1 + 1 + 1 * m_max_vertex_blending_mesh_count;
    pool_sizes[2].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[2].descriptorCount = 1 * m_max_material_count;
    pool_sizes[3].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[3].descriptorCount = 3 + 5 * m_max_material_count + 1 + 1 ; // ImGui_ImplVulkan_CreateDeviceObjects
    pool_sizes[4].type            = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pool_sizes[4].descriptorCount = 4 + 1 + 1 + 2+1 + 6;
    pool_sizes[5].type=VK_DESCRIPTOR_TYPE_SAMPLER;
    pool_sizes[5].descriptorCount=1;

    VkDescriptorPoolCreateInfo pool_info {};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    pool_info.pPoolSizes    = pool_sizes;
    pool_info.maxSets =
            1 + 1 + 1 + m_max_material_count + m_max_vertex_blending_mesh_count + 1 + 1; // +skybox + axis descriptor set
    pool_info.flags = 0U;

    if (vkCreateDescriptorPool(m_vulkan_context->_device, &pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("create descriptor pool");
    }

    return true;
}

