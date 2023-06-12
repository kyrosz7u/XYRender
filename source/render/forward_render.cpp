//
// Created by kyrosz7u on 2023/5/19.
//

#include "render/forward_render.h"
#include "render/renderpass/main_camera_renderpass.h"
#include "render/renderpass/ui_overlay_renderpass.h"
#include "core/logger/logger_macros.h"

using namespace RenderSystem;

void ForwardRender::initialize()
{
    m_render_command_info.command_buffer_list = &m_command_buffers;
    m_render_command_info.p_descriptor_pool   = &m_descriptor_pool;
    m_render_command_info.p_viewport          = &m_viewport;
    m_render_command_info.p_scissor           = &m_scissor;

    m_render_resource_info.p_render_submeshes        = &m_render_submeshes;
    m_render_resource_info.p_texture_descriptor_sets = &m_texture_descriptor_sets;
    m_render_resource_info.p_render_model_ubo_list   = &m_render_model_ubo_list;
    m_render_resource_info.p_render_per_frame_ubo    = &m_render_per_frame_ubo;
    m_render_resource_info.p_ui_overlay              = m_p_ui_overlay;
    m_render_resource_info.p_skybox_descriptor_set   = &m_skybox_descriptor_set;

    m_backup_targets.resize(1);
    setupBackupBuffer();
    setupRenderTargets();
    setupCommandBuffer();
    setupDescriptorPool();
    setViewport();
    setupRenderpass();

    setupRenderDescriptorSetLayout();
}

void ForwardRender::setupRenderTargets()
{
    int                          renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();
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

void ForwardRender::setupCommandBuffer()
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
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1U;
    command_buffer_allocate_info.commandPool        = m_command_pool;

    int renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();
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

void ForwardRender::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorTypes =
                                              {
                                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2},
                                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1},
                                                      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       2},
                                                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 11}
                                              };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorTypes.size());
    descriptorPoolInfo.pPoolSizes    = descriptorTypes.data();
    // NOTICE: the maxSets must be equal to the descriptorSets in all subpasses
    descriptorPoolInfo.maxSets       = 13;

    VK_CHECK_RESULT(vkCreateDescriptorPool(g_p_vulkan_context->_device,
                                           &descriptorPoolInfo,
                                           nullptr,
                                           &m_descriptor_pool))
}

void ForwardRender::setupBackupBuffer()
{
    m_backup_targets[0] = ImageAttachment{
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VulkanUtil::createImage(g_p_vulkan_context,
                            g_p_vulkan_context->_swapchain_extent.width,
                            g_p_vulkan_context->_swapchain_extent.height,
                            m_backup_targets[0].format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            m_backup_targets[0].image,
                            m_backup_targets[0].mem,
                            0,
                            1,
                            1);

    m_backup_targets[0].view = VulkanUtil::createImageView(g_p_vulkan_context,
                                                           m_backup_targets[0].image,
                                                           m_backup_targets[0].format,
                                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                                           VK_IMAGE_VIEW_TYPE_2D,
                                                           1,
                                                           1);
}

void ForwardRender::setViewport()
{
    uint32_t width  = g_p_vulkan_context->_swapchain_extent.width;
    uint32_t height = g_p_vulkan_context->_swapchain_extent.height;

    m_viewport = VkViewport({0, 0, (float) width, (float) height, 0, 1});
    m_scissor  = VkRect2D({{0,     0},
                           {width, height}});
}

void ForwardRender::setupRenderpass()
{
    m_render_passes.resize(_renderpass_count);

    m_render_passes[_main_camera_renderpass] = std::make_shared<MainCameraRenderPass>();
    m_render_passes[_ui_overlay_renderpass]  = std::make_shared<UIOverlayRenderPass>();

    MainCameraRenderPassInitInfo maincamera_renderpass_init_info;
    maincamera_renderpass_init_info.render_command_info  = &m_render_command_info;
    maincamera_renderpass_init_info.render_resource_info = &m_render_resource_info;
    maincamera_renderpass_init_info.render_targets       = &m_backup_targets;
    maincamera_renderpass_init_info.descriptor_pool      = &m_descriptor_pool;

    UIOverlayRenderPassInitInfo ui_overlay_renderpass_init_info;
    ui_overlay_renderpass_init_info.render_command_info  = &m_render_command_info;
    ui_overlay_renderpass_init_info.render_resource_info = &m_render_resource_info;
    ui_overlay_renderpass_init_info.render_targets       = &m_render_targets;
    ui_overlay_renderpass_init_info.descriptor_pool      = &m_descriptor_pool;
    ui_overlay_renderpass_init_info.in_color_attachment  = &m_backup_targets[0];

    m_render_passes[_main_camera_renderpass]->initialize(&maincamera_renderpass_init_info);
    m_render_passes[_ui_overlay_renderpass]->initialize(&ui_overlay_renderpass_init_info);
}

void ForwardRender::Tick()
{
    RenderBase::Tick();
    draw();
}

void ForwardRender::draw()
{
    uint32_t next_image_index = g_p_vulkan_context->getNextSwapchainImageIndex(
            std::bind(&ForwardRender::updateAfterSwapchainRecreate, this));
    if (next_image_index == -1)
    {
        LOG_INFO("next image index is -1");
        return;
    }
    vkResetCommandBuffer(m_command_buffers[next_image_index], 0);

    // begin command buffer
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    VkResult res_begin_command_buffer =
                     g_p_vulkan_context->_vkBeginCommandBuffer(m_command_buffers[next_image_index],
                                                               &command_buffer_begin_info);
    assert(VK_SUCCESS == res_begin_command_buffer);

    // record command buffer
    m_render_command_info.p_current_command_buffer = &m_command_buffers[next_image_index];

    m_render_passes[_main_camera_renderpass]->draw(0);
    m_render_passes[_ui_overlay_renderpass]->draw(next_image_index);

    // end command buffer
    VkResult res_end_command_buffer = g_p_vulkan_context->_vkEndCommandBuffer(m_command_buffers[next_image_index]);
    assert(VK_SUCCESS == res_end_command_buffer);

    g_p_vulkan_context->submitDrawSwapchainImageCmdBuffer(&m_command_buffers[next_image_index]);
    g_p_vulkan_context->presentSwapchainImage(next_image_index,
                                              std::bind(&ForwardRender::updateAfterSwapchainRecreate, this));
}

void ForwardRender::UpdateRenderSubMesh(const std::vector<RenderSubmesh> &_visible_submesh)
{
    m_render_submeshes = _visible_submesh;
}

void ForwardRender::UpdateRenderModelUBOList(std::vector<VulkanModelDefine> &model_matrix)
{
    m_render_model_ubo_list.ubo_data_list.resize(model_matrix.size());
    for (int i = 0; i < model_matrix.size(); ++i)
    {
        m_render_model_ubo_list.ubo_data_list[i].model = model_matrix[i].model;
    }
    m_render_model_ubo_list.ToGPU();
}

void ForwardRender::UpdateRenderPerFrameScenceUBO(VulkanPerFrameSceneDefine &per_frame_scene_define)
{
    m_render_per_frame_ubo.per_frame_ubo = per_frame_scene_define;
    m_render_per_frame_ubo.ToGPU();
}

void ForwardRender::setupRenderDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> texture_layout_bindings;
    texture_layout_bindings.resize(1);

    VkDescriptorSetLayoutBinding &texture_binding = texture_layout_bindings[0];
    texture_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    texture_binding.binding         = 0;
    texture_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo texture_descriptorSetLayoutCreateInfo;
    texture_descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    texture_descriptorSetLayoutCreateInfo.flags        = 0;
    texture_descriptorSetLayoutCreateInfo.pNext        = nullptr;
    texture_descriptorSetLayoutCreateInfo.bindingCount = texture_layout_bindings.size();
    texture_descriptorSetLayoutCreateInfo.pBindings    = texture_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &texture_descriptorSetLayoutCreateInfo,
                                                nullptr,
                                                &m_texture_descriptor_set_layout));

    std::vector<VkDescriptorSetLayoutBinding> skybox_layout_bindings;
    skybox_layout_bindings.resize(2);

    VkDescriptorSetLayoutBinding &skybox_ubodata_binding = skybox_layout_bindings[0];
    skybox_ubodata_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    skybox_ubodata_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    skybox_ubodata_binding.binding         = 0;
    skybox_ubodata_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &skybox_cubemap_binding = skybox_layout_bindings[1];
    skybox_cubemap_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skybox_cubemap_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    skybox_cubemap_binding.binding         = 1;
    skybox_cubemap_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo skybox_desc_set_layout_create_info;
    skybox_desc_set_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    skybox_desc_set_layout_create_info.flags        = 0;
    skybox_desc_set_layout_create_info.pNext        = nullptr;
    skybox_desc_set_layout_create_info.bindingCount = skybox_layout_bindings.size();
    skybox_desc_set_layout_create_info.pBindings    = skybox_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &skybox_desc_set_layout_create_info,
                                                nullptr,
                                                &m_skybox_descriptor_set_layout));

}

void ForwardRender::UpdateRenderTextures(const std::vector<Texture2DPtr> &_visible_textures)
{
    // wait for device idle
    // 很慢的，慎用
    vkDeviceWaitIdle(g_p_vulkan_context->_device);
    if (m_texture_descriptor_sets.size() > 0)
    {
        g_p_vulkan_context->_vkFreeDescriptorSets(g_p_vulkan_context->_device,
                                                  m_descriptor_pool,
                                                  m_texture_descriptor_sets.size(),
                                                  m_texture_descriptor_sets.data());
    }

    m_texture_descriptor_sets.resize(_visible_textures.size());

    for (int i = 0; i < m_texture_descriptor_sets.size(); ++i)
    {
        VkDescriptorSetAllocateInfo allocInfo{};

        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = m_descriptor_pool;
        // determines the number of info sets to be allocated from the pool.
        allocInfo.descriptorSetCount = 1;
        // 每个set的布局
        allocInfo.pSetLayouts        = &m_texture_descriptor_set_layout;

        if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                     &allocInfo,
                                     &m_texture_descriptor_sets[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate info sets!");
        }
    }

    assert(m_texture_descriptor_sets.size() == _visible_textures.size());

    for (int i = 0; i < _visible_textures.size(); ++i)
    {
//        VkDescriptorImageInfo imageInfo{};
//        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        imageInfo.imageView   = _visible_textures[i]->view;
//        imageInfo.sampler     = _visible_textures[i]->sampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = m_texture_descriptor_sets[i];
        descriptorWrite.dstBinding      = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo      = &_visible_textures[i]->info;

        vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                               1,
                               &descriptorWrite,
                               0,
                               nullptr);
    }
}

void ForwardRender::UpdateSkyboxTexture(const std::shared_ptr<TextureCube> skybox_texture)
{
    // wait for device idle
    // 很慢的，禁止频繁使用
    vkDeviceWaitIdle(g_p_vulkan_context->_device);
    if (m_skybox_descriptor_set != VK_NULL_HANDLE)
    {
        g_p_vulkan_context->_vkFreeDescriptorSets(g_p_vulkan_context->_device,
                                                  m_descriptor_pool,
                                                  1,
                                                  &m_skybox_descriptor_set);
    }
    VkDescriptorSetAllocateInfo allocInfo{};

    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_descriptor_pool;
    // determines the number of info sets to be allocated from the pool.
    allocInfo.descriptorSetCount = 1;
    // 每个set的布局
    allocInfo.pSetLayouts        = &m_skybox_descriptor_set_layout;

    if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_skybox_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate info sets!");
    }
    VkWriteDescriptorSet descriptor_set_writes[2];

    VkWriteDescriptorSet &ubo_descriptor_write = descriptor_set_writes[0];
    ubo_descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ubo_descriptor_write.pNext           = nullptr;
    ubo_descriptor_write.dstSet          = m_skybox_descriptor_set;
    ubo_descriptor_write.dstBinding      = 0;
    ubo_descriptor_write.dstArrayElement = 0;
    ubo_descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_descriptor_write.descriptorCount = 1;
    ubo_descriptor_write.pBufferInfo     = &m_render_per_frame_ubo.buffer_infos[RenderPerFrameUBO::_scene_info_block];

    VkWriteDescriptorSet &cube_descriptor_write = descriptor_set_writes[1];
    cube_descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cube_descriptor_write.pNext           = nullptr;    // 天坑！！！！！！，不设置为nullptr，会导致crash
    cube_descriptor_write.dstSet          = m_skybox_descriptor_set;
    cube_descriptor_write.dstBinding      = 1;
    cube_descriptor_write.dstArrayElement = 0;
    cube_descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cube_descriptor_write.descriptorCount = 1;
    cube_descriptor_write.pImageInfo      = &skybox_texture->info;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           1,
                           descriptor_set_writes,
                           0,
                           nullptr);
}

void ForwardRender::updateAfterSwapchainRecreate()
{
    vkDestroyImage(g_p_vulkan_context->_device, m_backup_targets[0].image, nullptr);
    vkDestroyImageView(g_p_vulkan_context->_device, m_backup_targets[0].view, nullptr);
    vkFreeMemory(g_p_vulkan_context->_device, m_backup_targets[0].mem, nullptr);

    setupBackupBuffer();
    setupRenderTargets();
    setViewport();
//    renderPass.updateAfterSwapchainRecreate();
    for (int i = 0; i < m_render_passes.size(); ++i)
    {
        m_render_passes[i]->updateAfterSwapchainRecreate();
    }
}

ForwardRender::~ForwardRender()
{
    g_p_vulkan_context->waitForFrameInFlightFence();
    vkDeviceWaitIdle(g_p_vulkan_context->_device);
    vkDestroyDescriptorSetLayout(g_p_vulkan_context->_device, m_texture_descriptor_set_layout, nullptr);
    vkDestroyDescriptorSetLayout(g_p_vulkan_context->_device, m_skybox_descriptor_set_layout, nullptr);

    vkDestroyCommandPool(g_p_vulkan_context->_device, m_command_pool, nullptr);
    vkDestroyDescriptorPool(g_p_vulkan_context->_device, m_descriptor_pool, nullptr);
}


