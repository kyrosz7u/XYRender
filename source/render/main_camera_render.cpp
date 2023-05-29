//
// Created by kyrosz7u on 2023/5/19.
//

#include "render/main_camera_render.h"
#include "logger/logger_macros.h"

using namespace RenderSystem;

void MainCameraRender::initialize()
{
    render_resource_info.p_viewport = &m_viewport;
    render_resource_info.p_scissor  = &m_scissor;

    render_resource_info

    setupRenderTargets();
    setupCommandBuffer();
    setupDescriptorPool();
    setViewport();
    setupRenderpass();
}

void MainCameraRender::setupRenderTargets()
{
    int renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();
    std::vector<ImageAttachment> targets_tmp;
    targets_tmp.resize(renderTarget_nums);

    for (int i = 0; i < renderTarget_nums; ++i)
    {
        targets_tmp[i] = ImageAttachment{
                g_p_vulkan_context->_swapchain_images[i],
                VK_NULL_HANDLE,
                g_p_vulkan_context->_swapchain_imageviews[i],
                g_p_vulkan_context->_swapchain_image_format,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    }
    m_render_targets.swap(targets_tmp);
}

void MainCameraRender::setupCommandBuffer()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = g_p_vulkan_context->_queue_indices.graphicsFamily.value();

    if (vkCreateCommandPool(g_p_vulkan_context->_device, &command_pool_create_info, NULL, &m_command_pool) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("vk create command pool");
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;

    int renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();
    m_command_buffers.resize(renderTarget_nums);
    for (uint32_t i = 0; i < renderTarget_nums; ++i)
    {
        command_buffer_allocate_info.commandPool = m_command_pool;

        if (vkAllocateCommandBuffers(g_p_vulkan_context->_device, &command_buffer_allocate_info,
                                     &m_command_buffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("vk allocate command buffers");
        }
    }
}

void MainCameraRender::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorTypes =
            {
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}
            };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorTypes.size());
    descriptorPoolInfo.pPoolSizes = descriptorTypes.data();
    descriptorPoolInfo.maxSets = 2;

    VK_CHECK_RESULT(vkCreateDescriptorPool(g_p_vulkan_context->_device,
                                           &descriptorPoolInfo,
                                           nullptr,
                                           &m_descriptor_pool))
}

void MainCameraRender::setViewport()
{
    uint32_t width = g_p_vulkan_context->_swapchain_extent.width;
    uint32_t height = g_p_vulkan_context->_swapchain_extent.height;

    m_viewport = VkViewport({0, 0, (float) width, (float) height, 0, 1});
    m_scissor = VkRect2D({{0,     0},
                          {width, height}});
}

void MainCameraRender::setupRenderpass()
{
    MainCameraRenderPassInitInfo maincamera_renderpass_init_info;

    maincamera_renderpass_init_info.render_command_info = &render_resource_info;
    maincamera_renderpass_init_info.render_resource_info = &render_resource_info;
    maincamera_renderpass_init_info.render_targets = &m_render_targets;
    maincamera_renderpass_init_info.descriptor_pool = &m_descriptor_pool;


    renderPass.initialize(&maincamera_renderpass_init_info);
}

void MainCameraRender::Tick()
{
    draw();
}

void MainCameraRender::draw()
{
    uint32_t next_image_index = g_p_vulkan_context->getNextSwapchainImageIndex(
            std::bind(&MainCameraRender::updateAfterSwapchainRecreate, this));
    if (next_image_index == -1)
    {
        LOG_INFO("next image index is -1");
        return;
    }
    vkResetCommandBuffer(m_command_buffers[next_image_index], 0);

    // begin command buffer
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    VkResult res_begin_command_buffer =
            g_p_vulkan_context->_vkBeginCommandBuffer(m_command_buffers[next_image_index], &command_buffer_begin_info);
    assert(VK_SUCCESS == res_begin_command_buffer);

    // record command buffer
    render_resource_info.p_current_command_buffer = &m_command_buffers[next_image_index];

    renderPass.draw(next_image_index);

    // end command buffer
    VkResult res_end_command_buffer = g_p_vulkan_context->_vkEndCommandBuffer(m_command_buffers[next_image_index]);
    assert(VK_SUCCESS == res_end_command_buffer);

    g_p_vulkan_context->submitDrawSwapchainImageCmdBuffer(&m_command_buffers[next_image_index]);
    g_p_vulkan_context->presentSwapchainImage(next_image_index,
                                              std::bind(&MainCameraRender::updateAfterSwapchainRecreate, this));
}

void MainCameraRender::loadSceneMeshes(std::vector<RenderMeshPtr> &visible_meshes)
{
    m_visible_meshes = visible_meshes;
    for(int i=0;i<m_visible_meshes.size();++i)
    {
        m_visible_meshes[i]->m_index_in_dynamic_buffer=i;
        auto model_matrix_in_buffer = static_cast<VulkanModelDefine*>(m_render_model_ubo_list.mapped_buffer_ptr);
        model_matrix_in_buffer[i].model = m_visible_meshes[i]->model_matrix;
    }
    m_render_model_ubo_list.ToGPU();
}

void MainCameraRender::updateAfterSwapchainRecreate()
{
    setupRenderTargets();
    setViewport();
    setupRenderpass();
}

MainCameraRender::~MainCameraRender()
{
    g_p_vulkan_context->waitForFrameInFlightFence();
    vkDestroyCommandPool(g_p_vulkan_context->_device, m_command_pool, nullptr);
    vkDestroyDescriptorPool(g_p_vulkan_context->_device, m_descriptor_pool, nullptr);
}


