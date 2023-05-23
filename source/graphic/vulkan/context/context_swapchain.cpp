#include "graphic/vulkan/vulkan_context.h"

using namespace VulkanAPI;


void VulkanContext::presentCurrentSwapchainImage(uint32_t current_swapchain_image_index)
{
    // present swapchain
    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &m_image_finished_for_presentation_semaphores[m_current_frame_index];
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &m_vulkan_context->_swapchain;
    present_info.pImageIndices      = &current_swapchain_image_index;

    VkResult present_result = vkQueuePresentKHR(m_vulkan_context->_present_queue, &present_info);
    if (VK_ERROR_OUT_OF_DATE_KHR == present_result || VK_SUBOPTIMAL_KHR == present_result)
    {
//        recreateSwapChain();
    }
    else
    {
        assert(VK_SUCCESS == present_result);
    }

    m_current_frame_index = (m_current_frame_index + 1) % m_max_frames_in_flight;
}

uint32_t VulkanContext::getCurrentSwapchainImageIndex()
{
    // sync device
    VkResult res_wait_for_fences = m_vulkan_context->_vkWaitForFences(
            m_vulkan_context->_device, 1, &m_is_frame_in_flight_fences[m_current_frame_index], VK_TRUE, UINT64_MAX);
    assert(VK_SUCCESS == res_wait_for_fences);

    uint32_t current_swapchain_image_index;
    VkResult acquire_image_result =
            vkAcquireNextImageKHR(m_vulkan_context->_device,
                                  m_vulkan_context->_swapchain,
                                  UINT64_MAX,
                                  m_image_available_for_render_semaphores[m_current_frame_index],
                                  VK_NULL_HANDLE,
                                  &current_swapchain_image_index);
    if (VK_ERROR_OUT_OF_DATE_KHR == acquire_image_result)
    {
        // recreateSwapChain();
        return -1;
    }
    else if (VK_SUBOPTIMAL_KHR == acquire_image_result)
    {
        // recreateSwapChain();

        // NULL submit to wait semaphore
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
        VkSubmitInfo         submit_info   = {};
        submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount     = 1;
        submit_info.pWaitSemaphores        = &m_image_available_for_render_semaphores[m_current_frame_index];
        submit_info.pWaitDstStageMask      = wait_stages;
        submit_info.commandBufferCount     = 0;
        submit_info.pCommandBuffers        = NULL;
        submit_info.signalSemaphoreCount   = 0;
        submit_info.pSignalSemaphores      = NULL;

        VkResult res_reset_fences = m_vulkan_context->_vkResetFences(
                m_vulkan_context->_device, 1, &m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_reset_fences);

        VkResult res_queue_submit = vkQueueSubmit(
                m_vulkan_context->_graphics_queue, 1, &submit_info, m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_queue_submit);

        m_current_frame_index = (m_current_frame_index + 1) % m_max_frames_in_flight;
        return -1;
    }
    else
    {
        assert(VK_SUCCESS == acquire_image_result);
    }

    // // begin command buffer
    // VkCommandBufferBeginInfo command_buffer_begin_info {};
    // command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // command_buffer_begin_info.flags            = 0;
    // command_buffer_begin_info.pInheritanceInfo = nullptr;

    // VkResult res_begin_command_buffer =
    //     m_vulkan_context._vkBeginCommandBuffer(m_command_buffers[m_current_frame_index], &command_buffer_begin_info);
    // assert(VK_SUCCESS == res_begin_command_buffer);

    // // end command buffer
    // VkResult res_end_command_buffer = m_vulkan_context._vkEndCommandBuffer(m_command_buffers[m_current_frame_index]);
    // assert(VK_SUCCESS == res_end_command_buffer);

    return current_swapchain_image_index;
}


void VulkanContext::presentCurrentSwapchainImage(uint32_t current_swapchain_image_index)
{
    // present swapchain
    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &m_image_finished_for_presentation_semaphores[m_current_frame_index];
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &m_vulkan_context->_swapchain;
    present_info.pImageIndices      = &current_swapchain_image_index;

    VkResult present_result = vkQueuePresentKHR(m_vulkan_context->_present_queue, &present_info);
    if (VK_ERROR_OUT_OF_DATE_KHR == present_result || VK_SUBOPTIMAL_KHR == present_result)
    {
//        recreateSwapChain();
    }
    else
    {
        assert(VK_SUCCESS == present_result);
    }

    m_current_frame_index = (m_current_frame_index + 1) % m_max_frames_in_flight;
}

void VulkanManager::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_vulkan_context->_window, &width, &height);
    while (width == 0 || height == 0) // minimized 0,0 ,pause for now
    {
        glfwGetFramebufferSize(m_vulkan_context->_window, &width, &height);
        glfwWaitEvents();
    }

    VkResult res_wait_for_fences = m_vulkan_context->_vkWaitForFences(
            m_vulkan_context->_device, m_max_frames_in_flight, m_is_frame_in_flight_fences, VK_TRUE, UINT64_MAX);
    assert(VK_SUCCESS == res_wait_for_fences);

    m_vulkan_context->clearSwapchain();
    m_vulkan_context->createSwapchain();
    m_vulkan_context->createSwapchainImageViews();
}


