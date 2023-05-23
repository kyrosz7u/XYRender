//
// Created by kyrosz7u on 2023/5/19.
//

#include "render/MainCameraRender.h"


void MainCameraRender::initialize()
{
    int renderTarget_nums=m_vulkan_context->_swapchain_images.size();
    renderTargets.resize(renderTarget_nums);

    for (int i=0;i<renderTarget_nums;++i)
    {
        renderTargets.push_back(ImageAttachment{
            m_vulkan_context->_swapchain_images[i],
            VK_NULL_HANDLE,
            m_vulkan_context->_swapchain_imageviews[i],
            m_vulkan_context->_swapchain_image_format,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR});
    }

    
}

void MainCameraRender::setupCommandBuffer()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext            = NULL;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    command_pool_create_info.queueFamilyIndex = m_vulkan_context->_queue_indices.graphicsFamily.value();

    if (vkCreateCommandPool(m_vulkan_context->_device, &command_pool_create_info, NULL, &commandPool) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("vk create command pool");
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info {};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;

    int renderTarget_nums=m_vulkan_context->_swapchain_images.size();
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

void MainCameraRender::setupRenderpass()
{
    RenderPassInitInfo renderpass_init_info;


}




