//
// Created by kyrosz7u on 2023/6/1.
//

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include "render/subpass/ui_pass.h"
#include "logger/logger_macros.h"

using namespace RenderSystem;
using namespace RenderSystem::SubPass;

void UIPass::initialize(SubPassInitInfo *subpass_init_info)
{
    auto ui_pass_init_info = static_cast<UIPassInitInfo *>(subpass_init_info);
    m_p_render_command_info  = ui_pass_init_info->p_render_command_info;
    m_p_render_resource_info = ui_pass_init_info->p_render_resource_info;
    m_subpass_index          = ui_pass_init_info->subpass_index;
    m_renderpass             = ui_pass_init_info->renderpass;

    initializeUIRenderBackend();
}

void UIPass::initializeUIRenderBackend()
{
    ImGui_ImplGlfw_InitForVulkan(g_p_vulkan_context->_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance       = g_p_vulkan_context->_instance;
    init_info.PhysicalDevice = g_p_vulkan_context->_physical_device;
    init_info.Device         = g_p_vulkan_context->_device;
    init_info.QueueFamily    = g_p_vulkan_context->_queue_indices.graphicsFamily.value();
    init_info.Queue          = g_p_vulkan_context->_graphics_queue;
    init_info.DescriptorPool = *m_p_render_command_info->p_descriptor_pool;
    init_info.Subpass        = m_subpass_index;

    // may be different from the real swapchain image count
    // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
    init_info.MinImageCount = 3;
    init_info.ImageCount    = 3;
    ImGui_ImplVulkan_Init(&init_info, m_renderpass);

    uploadFonts();
}

void UIPass::uploadFonts()
{
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = g_p_vulkan_context->_command_pool; // 为了提高性能，不使用渲染的command pool
    allocInfo.commandBufferCount = 1;

    VK_CHECK_RESULT(vkAllocateCommandBuffers(g_p_vulkan_context->_device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &command_buffer_begin_info))
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer))

    VkSubmitInfo submit_info = {};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &commandBuffer;

    VK_CHECK_RESULT(vkQueueSubmit(g_p_vulkan_context->_graphics_queue,
                                  1,
                                  &submit_info,
                                  VK_NULL_HANDLE))
    VK_CHECK_RESULT(vkDeviceWaitIdle(g_p_vulkan_context->_device))

    vkFreeCommandBuffers(g_p_vulkan_context->_device,
                         g_p_vulkan_context->_command_pool,
                         1,
                         &commandBuffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void UIPass::draw()
{
    auto current_ui = m_p_render_resource_info->p_ui_overlay.lock();

    if(current_ui!= nullptr && current_ui->isInitialized())
    {
        VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "IMGUI", {1.0f, 1.0f, 1.0f, 1.0f}};
        g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer, &label_info);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        current_ui->drawImGui();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                        *m_p_render_command_info->p_current_command_buffer);
        g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);
    }
}

void UIPass::updateAfterSwapchainRecreate()
{

}



