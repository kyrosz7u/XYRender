//
// Created by kyrosz7u on 2023/6/1.
//

#include "render/subpass/ui_pass.h"

using namespace RenderSystem::SubPass;

void UIPass::initialize(SubPassInitInfo *subpass_init_info)
{
    auto ui_pass_init_info        = static_cast<UIPassInitInfo *>(subpass_init_info);
         m_p_render_command_info  = ui_pass_init_info->render_command_info;
         m_p_render_resource_info = ui_pass_init_info->render_resource_info;
         subpass_index            = ui_pass_init_info->subpass_index;
         renderpass               = ui_pass_init_info->renderpass;

}

void UIPass::initializeUIRenderBackend()
{
    ImGui_ImplGlfw_InitForVulkan(std::static_pointer_cast<VulkanRHI>(m_rhi)->m_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_instance;
    init_info.PhysicalDevice            = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_physical_device;
    init_info.Device                    = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_device;
    init_info.QueueFamily               = m_rhi->getQueueFamilyIndices().graphics_family.value();
    init_info.Queue                     = ((VulkanQueue*)m_rhi->getGraphicsQueue())->getResource();
    init_info.DescriptorPool            = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_vk_descriptor_pool;
    init_info.Subpass                   = _main_camera_subpass_ui;
    
    // may be different from the real swapchain image count
    // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
    init_info.MinImageCount = 3;
    init_info.ImageCount    = 3;
    ImGui_ImplVulkan_Init(&init_info, ((VulkanRenderPass*)m_framebuffer.render_pass)->getResource());

    uploadFonts();
}

void UIPass::draw()
{
    g_p_vulkan_context->_vkCmdBindPipeline(*m_p_render_command_info->p_current_command_buffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    g_p_vulkan_context->_vkCmdSetViewport(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                          m_p_render_command_info->p_viewport);
    g_p_vulkan_context->_vkCmdSetScissor(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                         m_p_render_command_info->p_scissor);


    vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout,
                            0,
                            1,
                            &m_descriptorset_list[0].descriptor_set,
                            1,
                            nullptr);
    vkCmdDrawIndexed(*m_p_render_command_info->p_current_command_buffer, 3, 1, 0, 0, 0);

}

void CombineUIPass::updateAfterSwapchainRecreate()
{
    updateDescriptorSets();
}



