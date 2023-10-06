//
// Created by kyrosz7u on 2023/5/19.
//

#include "core/logger/logger_macros.h"
#include "render/forward_render.h"
#include "render/renderpass/directional_light_shadow_pass.h"
#include "render/renderpass/main_camera_forward_pass.h"
#include "render/renderpass/ui_overlay_pass.h"

using namespace RenderSystem;

void ForwardRender::initialize()
{
    m_render_command_info.p_descriptor_pool = &m_descriptor_pool;
    m_render_command_info.p_viewport        = &m_viewport;
    m_render_command_info.p_scissor         = &m_scissor;

    m_render_resource_info.p_render_submeshes              = &m_render_submeshes;
    m_render_resource_info.p_texture_descriptor_sets       = &m_texture_descriptor_sets;
    m_render_resource_info.p_render_model_ubo_list         = &m_render_model_ubo_list;
    m_render_resource_info.p_render_light_project_ubo_list = &m_render_light_project_ubo_list;
    m_render_resource_info.p_render_per_frame_ubo          = &m_render_per_frame_ubo;
    m_render_resource_info.p_ui_overlay                    = m_p_ui_overlay;
    m_render_resource_info.p_skybox_descriptor_set         = &m_skybox_descriptor_set;
    m_render_resource_info.p_directional_light_shadow_map_descriptor_set =
            &m_directional_light_shadow_set;

    m_backup_targets.resize(1);
    setupBackupBuffer();
    setupRenderTargets();
    setupCommandBuffer();
    setupDescriptorPool();
    setViewport();
    setupRenderDescriptorSetLayout();
}

void ForwardRender::postInitialize()
{
    setupRenderpass();
}

void ForwardRender::setupRenderTargets()
{
    uint32_t renderTarget_nums = g_p_vulkan_context->_swapchain_images.size();

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

void ForwardRender::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptor_types =
                                              {
                                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         3 + 1},
                                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 + 1},
                                                      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       2},
                                                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8 + 1 + 1 + 1}
                                              };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptor_types.size());
    descriptorPoolInfo.pPoolSizes    = descriptor_types.data();
    // NOTICE: the maxSets must be equal to the descriptorSets in all subpasses
    descriptorPoolInfo.maxSets       = 13 + 1;

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
                                                           VK_IMAGE_VIEW_TYPE_2D, 1);
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

    m_render_passes[_directional_light_shadowmap_renderpass] = std::make_shared<DirectionalLightShadowRenderPass>();
    m_render_passes[_main_camera_renderpass]                 = std::make_shared<MainCameraForwardRenderPass>();
    m_render_passes[_ui_overlay_renderpass]                  = std::make_shared<UIOverlayRenderPass>();

    DirectionalLightShadowRenderPassInitInfo directional_light_shadowmap_renderpass_init_info;
    directional_light_shadowmap_renderpass_init_info.render_command_info  = &m_render_command_info;
    directional_light_shadowmap_renderpass_init_info.render_resource_info = &m_render_resource_info;
    directional_light_shadowmap_renderpass_init_info.descriptor_pool      = &m_descriptor_pool;
    directional_light_shadowmap_renderpass_init_info.shadowmap_attachment = &m_directional_light_shadow;

    MainCameraForwardRenderPassInitInfo maincamera_renderpass_init_info;
    maincamera_renderpass_init_info.render_command_info  = &m_render_command_info;
    maincamera_renderpass_init_info.render_resource_info = &m_render_resource_info;
    maincamera_renderpass_init_info.descriptor_pool      = &m_descriptor_pool;
    maincamera_renderpass_init_info.render_targets       = &m_backup_targets;

    UIOverlayRenderPassInitInfo ui_overlay_renderpass_init_info;
    ui_overlay_renderpass_init_info.render_command_info  = &m_render_command_info;
    ui_overlay_renderpass_init_info.render_resource_info = &m_render_resource_info;
    ui_overlay_renderpass_init_info.descriptor_pool      = &m_descriptor_pool;
    ui_overlay_renderpass_init_info.render_targets       = &m_render_targets;
    ui_overlay_renderpass_init_info.in_color_attachment  = &m_backup_targets[0];

    m_render_passes[_directional_light_shadowmap_renderpass]->initialize(
            &directional_light_shadowmap_renderpass_init_info);
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

#ifdef MULTI_THREAD_RENDERING
    m_render_passes[_directional_light_shadowmap_renderpass]->drawMultiThreading(0, next_image_index);
    m_render_passes[_main_camera_renderpass]->drawMultiThreading(0, next_image_index);
#else
    m_render_passes[_directional_light_shadowmap_renderpass]->draw(0);
    m_render_passes[_main_camera_renderpass]->draw(0);
#endif
    m_render_passes[_ui_overlay_renderpass]->draw(next_image_index);

    // end command buffer
    VkResult res_end_command_buffer = g_p_vulkan_context->_vkEndCommandBuffer(m_command_buffers[next_image_index]);
    assert(VK_SUCCESS == res_end_command_buffer);

    g_p_vulkan_context->submitDrawSwapchainImageCmdBuffer(&m_command_buffers[next_image_index]);
    g_p_vulkan_context->presentSwapchainImage(next_image_index, [this]
    { updateAfterSwapchainRecreate(); });
}

void ForwardRender::UpdateRenderModelList(const std::vector<Scene::Model> &_visible_models,
                                          const std::vector<RenderSubmesh> &_visible_submeshes)
{
    if (m_render_model_ubo_list.ubo_data_list.size() != _visible_models.size())
    {
        m_render_model_ubo_list.ubo_data_list.resize(_visible_models.size());
    }

    for (int i = 0; i < _visible_models.size(); ++i)
    {
        m_render_model_ubo_list.ubo_data_list[i].model  = _visible_models[i].GetModelMatrix();
        m_render_model_ubo_list.ubo_data_list[i].normal = _visible_models[i].GetNormalMatrix();
    }

    m_render_submeshes.clear();
    for (int i = 0; i < _visible_submeshes.size(); ++i)
    {
        m_render_submeshes.push_back(_visible_submeshes[i]);
    }
}

void ForwardRender::UpdateRenderPerFrameScenceUBO(
        Math::Matrix4x4 proj_view,
        Math::Vector3 camera_pos,
        std::vector<Scene::DirectionLight> &directional_light_list)
{
    m_render_per_frame_ubo.scene_data_ubo.proj_view                = proj_view;
    m_render_per_frame_ubo.scene_data_ubo.camera_pos               = camera_pos;
    m_render_per_frame_ubo.scene_data_ubo.directional_light_number = directional_light_list.size();
    assert(directional_light_list.size() <= MAX_DIRECTIONAL_LIGHT_COUNT);
    for (int i = 0; i < directional_light_list.size(); ++i)
    {
        m_render_per_frame_ubo.directional_lights_ubo[i].intensity = directional_light_list[i].intensity;
        m_render_per_frame_ubo.directional_lights_ubo[i].color     = directional_light_list[i].color;
        m_render_per_frame_ubo.directional_lights_ubo[i].direction = -directional_light_list[i].transform.GetForward();
    }
}

void ForwardRender::UpdateLightProjectionList(std::vector<Scene::DirectionLight> &directional_light_list, const std::shared_ptr<Scene::Camera> &main_camera)
{
    if (m_render_light_project_ubo_list.ubo_data_list.size() != directional_light_list.size())
    {
        m_render_light_project_ubo_list.ubo_data_list.resize(directional_light_list.size());
    }

    for (int i = 0; i < directional_light_list.size(); ++i)
    {
        auto dummy_transform = directional_light_list[i].transform;
        auto forward         = dummy_transform.GetForward();
        auto projection      = Math::Matrix4x4::makeOrthogonalMatrix(m_render_resource_info.kDirectionalLightInfo.camera_width,
                                                                     m_render_resource_info.kDirectionalLightInfo.camera_height,
                                                                     m_render_resource_info.kDirectionalLightInfo.camera_near,
                                                                     m_render_resource_info.kDirectionalLightInfo.camera_far);

//        auto projection      = Math::Matrix4x4::makePerspectiveMatrix(90,1.33,1.0,50);

        forward.normalise();
        dummy_transform.position = Math::Vector3(10, 0, -10) - forward * 30;

        auto r_inverse   = getRotationMatrix(dummy_transform.rotation).transpose();
        auto t_inverse   = Matrix4x4::getTrans(-dummy_transform.position);
        auto view_matrix = r_inverse * t_inverse;

        m_render_light_project_ubo_list.ubo_data_list[i].light_proj = projection * view_matrix;
    }
}

void ForwardRender::FlushRenderbuffer()
{
    m_render_per_frame_ubo.ToGPU();
    m_render_model_ubo_list.ToGPU();
    m_render_light_project_ubo_list.ToGPU();
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

    std::vector<VkDescriptorSetLayoutBinding> light_project_layout_bindings;
    light_project_layout_bindings.resize(1);

    VkDescriptorSetLayoutBinding &light_project_binding = light_project_layout_bindings[0];
    light_project_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    light_project_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    light_project_binding.binding         = 0;
    light_project_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo light_project_desc_set_layout_create_info;
    light_project_desc_set_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    light_project_desc_set_layout_create_info.flags        = 0;
    light_project_desc_set_layout_create_info.pNext        = nullptr;
    light_project_desc_set_layout_create_info.bindingCount = light_project_layout_bindings.size();
    light_project_desc_set_layout_create_info.pBindings    = light_project_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &light_project_desc_set_layout_create_info,
                                                nullptr,
                                                &m_directional_light_shadow_set_layout));
}

void ForwardRender::SetupModelRenderTextures(const std::vector<Texture2DPtr> &_visible_textures)
{
    // wait for device idle
    // 很慢的，慎用
    vkDeviceWaitIdle(g_p_vulkan_context->_device);
    if (m_texture_descriptor_sets.size() != _visible_textures.size())
    {
        if (!m_texture_descriptor_sets.empty())
        {
            g_p_vulkan_context->_vkFreeDescriptorSets(g_p_vulkan_context->_device,
                                                      m_descriptor_pool,
                                                      m_texture_descriptor_sets.size(),
                                                      m_texture_descriptor_sets.data());
        }

        m_texture_descriptor_sets.resize(_visible_textures.size());
        if (!m_texture_descriptor_sets.empty())
        {
            std::vector<VkDescriptorSetLayout> layouts(m_texture_descriptor_sets.size(),
                                                       m_texture_descriptor_set_layout);

            VkDescriptorSetAllocateInfo texture_descriptor_set_allocate_info;
            texture_descriptor_set_allocate_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            texture_descriptor_set_allocate_info.pNext              = nullptr;
            texture_descriptor_set_allocate_info.descriptorPool     = m_descriptor_pool;
            texture_descriptor_set_allocate_info.descriptorSetCount = layouts.size();
            texture_descriptor_set_allocate_info.pSetLayouts        = layouts.data();

            VK_CHECK_RESULT(vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                                     &texture_descriptor_set_allocate_info,
                                                     m_texture_descriptor_sets.data()));
        }
    }

    assert(m_texture_descriptor_sets.size() == _visible_textures.size());

    for (int i = 0; i < _visible_textures.size(); ++i)
    {
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

void ForwardRender::SetupSkyboxTexture(const std::shared_ptr<TextureCube> &skybox_texture)
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
                           sizeof(descriptor_set_writes) / sizeof(descriptor_set_writes[0]),
                           descriptor_set_writes,
                           0,
                           nullptr);
}

void ForwardRender::SetupShadowMapTexture(std::vector<Scene::DirectionLight> &directional_light_list)
{
    uint32_t directional_light_num = directional_light_list.size();

    if (m_directional_light_shadow_set != VK_NULL_HANDLE)
    {
        g_p_vulkan_context->_vkFreeDescriptorSets(g_p_vulkan_context->_device,
                                                  m_descriptor_pool,
                                                  1,
                                                  &m_directional_light_shadow_set);
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &m_directional_light_shadow_set_layout;

    if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_directional_light_shadow_set) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate info sets!");
    }

    if (m_directional_light_shadow.image != VK_NULL_HANDLE)
    {
        m_directional_light_shadow.destroy();
    }

    m_directional_light_shadow.width       = m_render_resource_info.kDirectionalLightInfo.shadowmap_width;
    m_directional_light_shadow.height      = m_render_resource_info.kDirectionalLightInfo.shadowmap_height;
    m_directional_light_shadow.layer_count = directional_light_num;
    m_directional_light_shadow.format      = m_render_resource_info.kDirectionalLightInfo.depth_format;
    m_directional_light_shadow.layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    m_directional_light_shadow.usage =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    m_directional_light_shadow.aspect    = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_directional_light_shadow.view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    m_directional_light_shadow.init();

    VkDescriptorImageInfo info;

    info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    info.imageView   = m_directional_light_shadow.view;
    info.sampler     = VulkanUtil::getOrCreateDepthSampler(g_p_vulkan_context);

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet          = m_directional_light_shadow_set;
    descriptor_write.dstBinding      = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo      = &info;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           1,
                           &descriptor_write,
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

    for (int i = 0; i < m_render_passes.size(); ++i)
    {
        m_render_passes[i]->updateAfterSwapchainRecreate();
    }
}

void ForwardRender::destroy()
{
    g_p_vulkan_context->waitForFrameInFlightFence();
    vkDeviceWaitIdle(g_p_vulkan_context->_device);
    vkDestroyDescriptorSetLayout(g_p_vulkan_context->_device, m_texture_descriptor_set_layout, nullptr);
    vkDestroyDescriptorSetLayout(g_p_vulkan_context->_device, m_skybox_descriptor_set_layout, nullptr);

    vkDestroyCommandPool(g_p_vulkan_context->_device, m_command_pool, nullptr);
    vkDestroyDescriptorPool(g_p_vulkan_context->_device, m_descriptor_pool, nullptr);
}

