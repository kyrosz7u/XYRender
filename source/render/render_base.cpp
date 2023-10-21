#include "render/render_base.h"
#include "render/common_define.h"

namespace RenderSystem
{
    // 渲染器全局变量定义
    std::shared_ptr<VulkanContext>            g_p_vulkan_context = nullptr;
    const uint32_t DEFAULT_THREAD_COUNT = 4;
    const uint32_t MESH_DRAW_THREAD_NUM = 4;

    // 初始化渲染器全局变量
    void RenderBase::setupGlobally(GLFWwindow *window)
    {
        g_p_vulkan_context = std::make_shared<VulkanContext>();
        g_p_vulkan_context->initialize(window);
    }

    void RenderBase::setupCommandBuffer()
    {
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.flags =
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = g_p_vulkan_context->_queue_indices.graphicsFamily.value();

        if (vkCreateCommandPool(g_p_vulkan_context->_device,
                                &command_pool_create_info,
                                nullptr,
                                &m_command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("vk create command pool");
        }

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1U;
        command_buffer_allocate_info.commandPool        = m_command_pool;

        uint32_t renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();
        m_command_buffers.resize(renderTarget_nums);
        for (uint32_t i = 0; i < renderTarget_nums; ++i)
        {
            if (vkAllocateCommandBuffers(g_p_vulkan_context->_device, &command_buffer_allocate_info,
                                         &m_command_buffers[i]) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("vk allocate command buffers");
            }
        }
    }
}






