//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "core/threadpool.h"
#include "render/common_define.h"
#include "render/subpass/subpass_base.h"

namespace RenderSystem
{
    using namespace VulkanAPI;

    class RenderPassBase;

    typedef std::shared_ptr<RenderPassBase> RenderPassPtr;

    class RenderPassBase
    {
    public:


        RenderPassBase()
        {
            assert(g_p_vulkan_context != nullptr);
        }

        static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
        {
            g_p_vulkan_context = vulkanContext;
        }

        void setupMultiThreading(uint32_t thread_count=DEFAULT_THREAD_COUNT)
        {
            m_thread_pool.setThreadCount(thread_count);
            m_thread_data.resize(thread_count);

            for(int i = 0; i < thread_count; i++)
            {
                m_thread_data[i].p_thread_pool = &m_thread_pool;
                m_thread_data[i].command_buffers.resize(g_p_vulkan_context->_swapchain_images.size());

                VkCommandPoolCreateInfo command_pool_create_info;
                command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                command_pool_create_info.pNext            = nullptr;
                command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                command_pool_create_info.queueFamilyIndex = g_p_vulkan_context->_queue_indices.graphicsFamily.value();

                if (vkCreateCommandPool(g_p_vulkan_context->_device,
                                        &command_pool_create_info,
                                        nullptr,
                                        &m_thread_data[i].secondary_command_pool) != VK_SUCCESS)
                {
                    throw std::runtime_error("vk create secondary command pool");
                }

                VkCommandBufferAllocateInfo command_buffer_allocate_info{};
                command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                command_buffer_allocate_info.commandBufferCount = m_thread_data[i].command_buffers.size();
                command_buffer_allocate_info.commandPool        = m_thread_data[i].secondary_command_pool;

                if (vkAllocateCommandBuffers(g_p_vulkan_context->_device, &command_buffer_allocate_info,
                                             m_thread_data[i].command_buffers.data()) != VK_SUCCESS)
                {
                    throw std::runtime_error("vk allocate command buffers");
                }
            }
        }

        virtual void initialize(RenderPassInitInfo *renderpass_init_info) = 0;

        virtual void draw(uint32_t render_image_index) = 0;

        virtual void drawMultiThreading(uint32_t render_target_index, uint32_t command_buffer_index)
        {
            throw std::runtime_error("drawMultiThreading not implemented");
        }

        virtual void updateAfterSwapchainRecreate() = 0;

    protected:
        friend struct ImageAttachment;

        virtual void setupSubpass() = 0;

        RenderCommandInfo            *m_p_render_command_info;
        RenderGlobalResourceInfo     *m_p_render_resource_info;
        std::vector<ImageAttachment> *m_p_render_targets; // [target_index][render_image_index]

        VkRenderPass                                       m_renderpass;
        std::vector<ImageAttachment>                       m_renderpass_attachments;
        std::vector<VkFramebuffer>                         m_framebuffer_per_rendertarget;
        std::vector<std::shared_ptr<SubPass::SubPassBase>> m_subpass_list;

        //multi threading support
        ThreadPool                    m_thread_pool;
        std::vector<RenderThreadData> m_thread_data;
    };
}

#endif //VULKANRENDER_VULKAN_RENDERPASS_H
