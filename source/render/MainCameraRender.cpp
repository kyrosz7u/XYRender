//
// Created by kyrosz7u on 2023/5/19.
//

#include "render/MainCameraRender.h"
#include "macros.h"

void MainCameraRender::initialize()
{
    int renderTarget_nums = m_vulkan_context->_swapchain_images.size();
    renderTargets.resize(renderTarget_nums);

    for (int i = 0; i < renderTarget_nums; ++i)
    {
        renderTargets[i] = ImageAttachment{
            m_vulkan_context->_swapchain_images[i],
            VK_NULL_HANDLE,
            m_vulkan_context->_swapchain_imageviews[i],
            m_vulkan_context->_swapchain_image_format,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    }

    setupCommandBuffer();
    setViewport();
    setupRenderpass();
}

void MainCameraRender::setupCommandBuffer()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = m_vulkan_context->_queue_indices.graphicsFamily.value();

    if (vkCreateCommandPool(m_vulkan_context->_device, &command_pool_create_info, NULL, &commandPool) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("vk create command pool");
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;

    int renderTarget_nums = m_vulkan_context->_swapchain_images.size();
    m_command_buffers.resize(renderTarget_nums);
    for (uint32_t i = 0; i < renderTarget_nums; ++i)
    {
        command_buffer_allocate_info.commandPool = commandPool;

        if (vkAllocateCommandBuffers(m_vulkan_context->_device, &command_buffer_allocate_info, &m_command_buffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk allocate command buffers");
        }
    }
}

void MainCameraRender::setViewport()
{
    uint32_t width = m_vulkan_context->_swapchain_extent.width;
    uint32_t height = m_vulkan_context->_swapchain_extent.height;

    m_viewport = VkViewport({0, 0, (float)width, (float)height, 0, 1});
    m_scissor = VkRect2D({{0, 0}, {width, height}});

    m_render_command_info._p_viewport = &m_viewport;
    m_render_command_info._p_scissor = &m_scissor;
}

void MainCameraRender::setupRenderpass()
{
    RenderPassInitInfo renderpass_init_info;
    renderpass_init_info.render_command_info = &m_render_command_info;

    std::vector<std::vector<ImageAttachment>> renderpass_targets;
    renderpass_targets.push_back(renderTargets);

    renderpass_init_info.render_targets.swap(renderpass_targets);

    renderPass.initialize(&renderpass_init_info);
}

void MainCameraRender::Tick()
{
    draw();
}

void MainCameraRender::draw()
{
    uint32_t next_image_index = m_vulkan_context->getNextSwapchainImageIndex();
    vkResetCommandBuffer(m_command_buffers[m_vulkan_context->m_current_frame_index], 0);

    // begin command buffer
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    VkResult res_begin_command_buffer =
        m_vulkan_context->_vkBeginCommandBuffer(m_command_buffers[m_vulkan_context->m_current_frame_index], &command_buffer_begin_info);
    assert(VK_SUCCESS == res_begin_command_buffer);

    // record command buffer
    m_render_command_info._p_current_command_buffer = &m_command_buffers[m_vulkan_context->m_current_frame_index];

    renderPass.draw(next_image_index);
//    LOG_INFO("draw frame: ", m_vulkan_context->m_current_frame_index);

    // end command buffer
    VkResult res_end_command_buffer = m_vulkan_context->_vkEndCommandBuffer(m_command_buffers[m_vulkan_context->m_current_frame_index]);
    assert(VK_SUCCESS == res_end_command_buffer);

    m_vulkan_context->submitDrawSwapchainImageCmdBuffer(&m_command_buffers[m_vulkan_context->m_current_frame_index]);
    m_vulkan_context->presentSwapchainImage(next_image_index);
}
