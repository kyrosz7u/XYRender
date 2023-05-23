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


