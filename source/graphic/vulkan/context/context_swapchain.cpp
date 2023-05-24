#include "graphic/vulkan/vulkan_context.h"

using namespace VulkanAPI;

void VulkanContext::initSemaphoreObjects()
{
    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

    for (uint32_t i = 0; i < m_max_frames_in_flight; i++)
    {
        assert(vkCreateSemaphore(_device,
                              &semaphore_create_info,
                              nullptr,
                              &m_image_available_for_render_semaphores[i]) == VK_SUCCESS);
        assert(vkCreateSemaphore(_device,
                              &semaphore_create_info,
                              nullptr,
                              &m_image_finished_for_presentation_semaphores[i]) == VK_SUCCESS);
        assert(vkCreateFence(_device, &fence_create_info, nullptr, &m_is_frame_in_flight_fences[i]) == VK_SUCCESS);
    }
}

uint32_t VulkanContext::getNextSwapchainImageIndex()
{
    // sync device
    // VkResult res_wait_for_fences = _vkWaitForFences(
    //         _device, 1, &m_is_frame_in_flight_fences[m_current_frame_index], VK_TRUE, UINT64_MAX);
    // assert(VK_SUCCESS == res_wait_for_fences);

    uint32_t next_swapchain_image_index;
    VkResult acquire_image_result =
            vkAcquireNextImageKHR(_device,
                                  _swapchain,
                                  UINT64_MAX,
                                  m_image_available_for_render_semaphores[m_current_frame_index],
                                  VK_NULL_HANDLE,
                                  &next_swapchain_image_index);
    if (acquire_image_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // recreateSwapChain();
        return -1;
    }
    else if (acquire_image_result == VK_SUBOPTIMAL_KHR)
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

        VkResult res_reset_fences = _vkResetFences(
                _device, 1, &m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_reset_fences);

        VkResult res_queue_submit = vkQueueSubmit(
                _graphics_queue, 1, &submit_info, m_is_frame_in_flight_fences[m_current_frame_index]);
        assert(VK_SUCCESS == res_queue_submit);

        m_current_frame_index = (m_current_frame_index + 1) % m_max_frames_in_flight;
        return -1;
    }
    else
    {
        assert(acquire_image_result == VK_SUCCESS);
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

    return next_swapchain_image_index;
}

void VulkanContext::submitDrawSwapchainImageCmdBuffer(VkCommandBuffer* p_command_buffer)
{
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo         submit_info   = {};
    submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount     = 1;
    submit_info.pWaitSemaphores        = &m_image_available_for_render_semaphores[m_current_frame_index];
    submit_info.pWaitDstStageMask      = wait_stages;
    submit_info.commandBufferCount     = 1;
    submit_info.pCommandBuffers        = p_command_buffer;
    submit_info.signalSemaphoreCount   = 1;
    submit_info.pSignalSemaphores      = &m_image_finished_for_presentation_semaphores[m_current_frame_index];

    VkResult res_reset_fences = _vkResetFences(
            _device, 1, &m_is_frame_in_flight_fences[m_current_frame_index]);
    assert(VK_SUCCESS == res_reset_fences);

    VkResult res_queue_submit = vkQueueSubmit(
            _graphics_queue, 1, &submit_info, m_is_frame_in_flight_fences[m_current_frame_index]);
    assert(VK_SUCCESS == res_queue_submit);
}

void VulkanContext::presentSwapchainImage(uint32_t swapchain_image_index)
{
    // present swapchain
    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &m_image_finished_for_presentation_semaphores[m_current_frame_index];
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &_swapchain;
    present_info.pImageIndices      = &swapchain_image_index;

    VkResult present_result = vkQueuePresentKHR(_present_queue, &present_info);
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

void VulkanContext::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) // minimized 0,0 ,pause for now
    {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    VkResult res_wait_for_fences = _vkWaitForFences(
            _device, m_max_frames_in_flight, m_is_frame_in_flight_fences, VK_TRUE, UINT64_MAX);
    assert(VK_SUCCESS == res_wait_for_fences);

    clearSwapchain();
    createSwapchain();
    createSwapchainImageViews();
}


