//
// Created by kyrosz7u on 2023/5/22.
//


#include "core/graphic/vulkan/vulkan_utils.h"
#include "render/subpass/mesh_gbuffer.h"
#include "render/subpass/defer_light.h"
#include "render/subpass/skybox.h"
#include "render/renderpass/main_camera_defer_pass.h"

#include "mesh_gbuffer_vert.h"
#include "mesh_gbuffer_frag.h"
#include "full_screen_vert.h"
#include "defer_lighting_frag.h"
#include "skybox_vert.h"
#include "skybox_frag.h"

using namespace RenderSystem;

void MainCameraDeferRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    auto main_camera_renderpass_init_info = static_cast<MainCameraDeferRenderPassInitInfo *>(renderpass_init_info);
    m_p_render_command_info  = main_camera_renderpass_init_info->render_command_info;
    m_p_render_resource_info = main_camera_renderpass_init_info->render_resource_info;
    m_p_render_targets       = main_camera_renderpass_init_info->render_targets;

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
#ifdef MULTI_THREAD_RENDERING
    setupMultiThreading(MESH_DRAW_THREAD_NUM);
#endif
}

void MainCameraDeferRenderPass::setupRenderpassAttachments()
{
    assert(m_p_render_targets != nullptr);
    assert(m_p_render_targets->size() > 0);

    m_renderpass_attachments[_main_camera_defer_gbuffer_color_attachment].format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    m_renderpass_attachments[_main_camera_defer_gbuffer_color_attachment].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderpass_attachments[_main_camera_defer_gbuffer_normal_attachment].format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    m_renderpass_attachments[_main_camera_defer_gbuffer_normal_attachment].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderpass_attachments[_main_camera_defer_gbuffer_position_attachment].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_renderpass_attachments[_main_camera_defer_gbuffer_position_attachment].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderpass_attachments[_main_camera_defer_color_attachment].format = (*m_p_render_targets)[0].format;
    m_renderpass_attachments[_main_camera_defer_color_attachment].layout = (*m_p_render_targets)[0].layout;

    m_renderpass_attachments[_main_camera_defer_depth_attachment].format = g_p_vulkan_context->findDepthFormat();
    m_renderpass_attachments[_main_camera_defer_depth_attachment].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (int i = 0; i < m_renderpass_attachments.size() - 2; ++i)
    {
        auto &renderpass_attachment = m_renderpass_attachments[i];
        VulkanUtil::createImage(g_p_vulkan_context,
                                g_p_vulkan_context->_swapchain_extent.width,
                                g_p_vulkan_context->_swapchain_extent.height,
                                renderpass_attachment.format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                renderpass_attachment.image,
                                renderpass_attachment.mem,
                                0,
                                1,
                                1);

        renderpass_attachment.view = VulkanUtil::createImageView(g_p_vulkan_context,
                                                                 renderpass_attachment.image,
                                                                 renderpass_attachment.format,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                                                 VK_IMAGE_VIEW_TYPE_2D,
                                                                 1);
    }

    m_renderpass_attachments[_main_camera_defer_color_attachment].format = (*m_p_render_targets)[0].format;
    m_renderpass_attachments[_main_camera_defer_color_attachment].layout = (*m_p_render_targets)[0].layout;

    auto &depth_attachment = m_renderpass_attachments[_main_camera_defer_depth_attachment];
    VulkanUtil::createImage(g_p_vulkan_context,
                            g_p_vulkan_context->_swapchain_extent.width,
                            g_p_vulkan_context->_swapchain_extent.height,
                            depth_attachment.format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            depth_attachment.image,
                            depth_attachment.mem,
                            0,
                            1,
                            1);

    depth_attachment.view = VulkanUtil::createImageView(g_p_vulkan_context,
                                                        depth_attachment.image,
                                                        depth_attachment.format,
                                                        VK_IMAGE_ASPECT_DEPTH_BIT,
                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                        1);

}

void MainCameraDeferRenderPass::setupRenderPass()
{
    VkAttachmentDescription attachments[_main_camera_defer_attachment_count] = {};

    VkAttachmentDescription &gbuffer_color_attachment_description = attachments[_main_camera_defer_gbuffer_color_attachment];
    gbuffer_color_attachment_description.format         = m_renderpass_attachments[_main_camera_defer_gbuffer_color_attachment].format;
    gbuffer_color_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    gbuffer_color_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    gbuffer_color_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_color_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    gbuffer_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_color_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    gbuffer_color_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription &gbuffer_normal_attachment_description = attachments[_main_camera_defer_gbuffer_normal_attachment];
    gbuffer_normal_attachment_description.format         = m_renderpass_attachments[_main_camera_defer_gbuffer_normal_attachment].format;
    gbuffer_normal_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    gbuffer_normal_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    gbuffer_normal_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_normal_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    gbuffer_normal_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_normal_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    gbuffer_normal_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription &gbuffer_worldpos_attachment_description = attachments[_main_camera_defer_gbuffer_position_attachment];
    gbuffer_worldpos_attachment_description.format         = m_renderpass_attachments[_main_camera_defer_gbuffer_position_attachment].format;
    gbuffer_worldpos_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    gbuffer_worldpos_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    gbuffer_worldpos_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_worldpos_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    gbuffer_worldpos_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    gbuffer_worldpos_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    gbuffer_worldpos_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription &framebuffer_image_attachment_description = attachments[_main_camera_defer_color_attachment];
    framebuffer_image_attachment_description.format         = m_renderpass_attachments[_main_camera_defer_color_attachment].format;
    framebuffer_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    framebuffer_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    framebuffer_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    framebuffer_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    framebuffer_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // renderpass初始化和结束时图像的布局
    framebuffer_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    framebuffer_image_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_defer_color_attachment].layout;

    VkAttachmentDescription &depth_attachment_description = attachments[_main_camera_defer_depth_attachment];
    depth_attachment_description.format         = m_renderpass_attachments[_main_camera_defer_depth_attachment].format;
    depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_defer_depth_attachment].layout;

    VkSubpassDescription subpasses[_main_camera_subpass_count] = {};

    VkAttachmentReference gbuffer_attachments_reference[3] = {};
    gbuffer_attachments_reference[0].attachment =
            &gbuffer_color_attachment_description - attachments;
    // 指定在subpass执行时，管线访问图像的布局
    gbuffer_attachments_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    gbuffer_attachments_reference[1].attachment =
            &gbuffer_normal_attachment_description - attachments;
    gbuffer_attachments_reference[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    gbuffer_attachments_reference[2].attachment =
            &gbuffer_worldpos_attachment_description - attachments;
    gbuffer_attachments_reference[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference{};
    depth_attachment_reference.attachment = &depth_attachment_description - attachments;
    depth_attachment_reference.layout     = depth_attachment_description.finalLayout;

    VkSubpassDescription &gbuffer_pass = subpasses[_main_camera_gbuffer_subpass];
    gbuffer_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    gbuffer_pass.colorAttachmentCount =
            sizeof(gbuffer_attachments_reference) / sizeof(gbuffer_attachments_reference[0]);
    gbuffer_pass.pColorAttachments       = &gbuffer_attachments_reference[0];
    gbuffer_pass.pDepthStencilAttachment = &depth_attachment_reference;
    gbuffer_pass.preserveAttachmentCount = 0;
    gbuffer_pass.pPreserveAttachments    = nullptr;

    VkAttachmentReference defer_lighting_input_attachment_description[_main_camera_defer_attachment_count - 2];

    defer_lighting_input_attachment_description[0].attachment = &gbuffer_color_attachment_description - attachments;
    defer_lighting_input_attachment_description[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    defer_lighting_input_attachment_description[1].attachment = &gbuffer_normal_attachment_description - attachments;
    defer_lighting_input_attachment_description[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    defer_lighting_input_attachment_description[2].attachment = &gbuffer_worldpos_attachment_description - attachments;
    defer_lighting_input_attachment_description[2].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference framebuffer_image_attachment_reference{};
    framebuffer_image_attachment_reference.attachment = &framebuffer_image_attachment_description - attachments;
    framebuffer_image_attachment_reference.layout     = framebuffer_image_attachment_description.finalLayout;

    VkSubpassDescription &defer_lighting_pass = subpasses[_main_camera_defer_lighting_subpass];
    defer_lighting_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    defer_lighting_pass.inputAttachmentCount    = sizeof(defer_lighting_input_attachment_description) /
                                                  sizeof(defer_lighting_input_attachment_description[0]);
    defer_lighting_pass.pInputAttachments       = &defer_lighting_input_attachment_description[0];
    defer_lighting_pass.colorAttachmentCount    = 1;
    defer_lighting_pass.pColorAttachments       = &framebuffer_image_attachment_reference;
    defer_lighting_pass.pDepthStencilAttachment = nullptr;
    defer_lighting_pass.preserveAttachmentCount = 0;
    defer_lighting_pass.pPreserveAttachments    = nullptr;

    VkSubpassDescription &skybox_pass = subpasses[_main_camera_skybox_subpass];
    skybox_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    skybox_pass.colorAttachmentCount    = 1;
    skybox_pass.pColorAttachments       = &framebuffer_image_attachment_reference;
    skybox_pass.pDepthStencilAttachment = &depth_attachment_reference;
    skybox_pass.preserveAttachmentCount = 0;
    skybox_pass.pPreserveAttachments    = nullptr;

    VkSubpassDependency dependencies[4];

    VkSubpassDependency &gbuffer_depend_external = dependencies[0];
    gbuffer_depend_external.srcSubpass      = VK_SUBPASS_EXTERNAL;
    gbuffer_depend_external.dstSubpass      = _main_camera_gbuffer_subpass;
    gbuffer_depend_external.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    gbuffer_depend_external.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    gbuffer_depend_external.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    gbuffer_depend_external.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    gbuffer_depend_external.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDependency &defer_lighting_depend_on_gbuffer = dependencies[1];
    defer_lighting_depend_on_gbuffer.srcSubpass      = _main_camera_gbuffer_subpass;
    defer_lighting_depend_on_gbuffer.dstSubpass      = _main_camera_defer_lighting_subpass;
    defer_lighting_depend_on_gbuffer.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    defer_lighting_depend_on_gbuffer.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    defer_lighting_depend_on_gbuffer.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    defer_lighting_depend_on_gbuffer.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    defer_lighting_depend_on_gbuffer.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDependency &skybox_pass_depend_on_lighting = dependencies[2];
    skybox_pass_depend_on_lighting.srcSubpass      = _main_camera_defer_lighting_subpass;
    skybox_pass_depend_on_lighting.dstSubpass      = _main_camera_skybox_subpass;
    skybox_pass_depend_on_lighting.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    skybox_pass_depend_on_lighting.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    skybox_pass_depend_on_lighting.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    skybox_pass_depend_on_lighting.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    skybox_pass_depend_on_lighting.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDependency &external_depend_on_skybox = dependencies[3];
    external_depend_on_skybox.srcSubpass      = _main_camera_skybox_subpass;
    external_depend_on_skybox.dstSubpass      = VK_SUBPASS_EXTERNAL;
    external_depend_on_skybox.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    external_depend_on_skybox.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    external_depend_on_skybox.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    external_depend_on_skybox.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    external_depend_on_skybox.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


    VkRenderPassCreateInfo renderpass_create_info{};
    renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
    renderpass_create_info.pAttachments    = attachments;
    renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
    renderpass_create_info.pSubpasses      = subpasses;
    renderpass_create_info.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
    renderpass_create_info.pDependencies   = dependencies;

    if (vkCreateRenderPass(
            g_p_vulkan_context->_device, &renderpass_create_info,
            nullptr, &m_renderpass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create main camera render pass");
    }
}

void MainCameraDeferRenderPass::setupFrameBuffer()
{
    int targetCount = (*m_p_render_targets).size();

    assert(targetCount > 0);

    m_framebuffer_per_rendertarget.resize(targetCount);

    // create frame buffer for every imageview
    for (size_t i = 0; i < targetCount; i++)
    {
        std::vector<VkImageView> framebuffer_attachments;
        framebuffer_attachments.resize(_main_camera_defer_attachment_count);
        framebuffer_attachments[_main_camera_defer_gbuffer_color_attachment]    =
                m_renderpass_attachments[_main_camera_defer_gbuffer_color_attachment].view;
        framebuffer_attachments[_main_camera_defer_gbuffer_normal_attachment]   =
                m_renderpass_attachments[_main_camera_defer_gbuffer_normal_attachment].view;
        framebuffer_attachments[_main_camera_defer_gbuffer_position_attachment] =
                m_renderpass_attachments[_main_camera_defer_gbuffer_position_attachment].view;
        framebuffer_attachments[_main_camera_defer_color_attachment] = (*m_p_render_targets)[i].view;
        framebuffer_attachments[_main_camera_defer_depth_attachment] =
                m_renderpass_attachments[_main_camera_defer_depth_attachment].view;

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags           = 0U;
        framebuffer_create_info.renderPass      = m_renderpass;
        framebuffer_create_info.attachmentCount = framebuffer_attachments.size();
        framebuffer_create_info.pAttachments    = framebuffer_attachments.data();
        framebuffer_create_info.width           = g_p_vulkan_context->_swapchain_extent.width;
        framebuffer_create_info.height          = g_p_vulkan_context->_swapchain_extent.height;
        framebuffer_create_info.layers          = 1;

        if (vkCreateFramebuffer(
                g_p_vulkan_context->_device, &framebuffer_create_info, nullptr,
                &m_framebuffer_per_rendertarget[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create main camera framebuffer");
        }
    }
}

void MainCameraDeferRenderPass::setupSubpass()
{
    SubPass::MeshGBufferPassInitInfo gbuffer_pass_init_info{};
    gbuffer_pass_init_info.p_render_command_info  = m_p_render_command_info;
    gbuffer_pass_init_info.p_render_resource_info = m_p_render_resource_info;
    gbuffer_pass_init_info.renderpass             = m_renderpass;
    gbuffer_pass_init_info.subpass_index          = _main_camera_gbuffer_subpass;

    m_subpass_list[_main_camera_gbuffer_subpass] = std::make_shared<SubPass::MeshGBufferPass>();
    m_subpass_list[_main_camera_gbuffer_subpass]->setShader(SubPass::VERTEX_SHADER, MESH_GBUFFER_VERT);
    m_subpass_list[_main_camera_gbuffer_subpass]->setShader(SubPass::FRAGMENT_SHADER, MESH_GBUFFER_FRAG);


    SubPass::DeferLightPassInitInfo lighting_pass_init_info{};
    lighting_pass_init_info.p_render_command_info       = m_p_render_command_info;
    lighting_pass_init_info.p_render_resource_info      = m_p_render_resource_info;
    lighting_pass_init_info.renderpass                  = m_renderpass;
    lighting_pass_init_info.subpass_index               = _main_camera_defer_lighting_subpass;
    lighting_pass_init_info.gbuffer_color_attachment    = &m_renderpass_attachments[_main_camera_defer_gbuffer_color_attachment];
    lighting_pass_init_info.gbuffer_normal_attachment   = &m_renderpass_attachments[_main_camera_defer_gbuffer_normal_attachment];
    lighting_pass_init_info.gbuffer_position_attachment = &m_renderpass_attachments[_main_camera_defer_gbuffer_position_attachment];

    m_subpass_list[_main_camera_defer_lighting_subpass] = std::make_shared<SubPass::DeferLightPass>();
    m_subpass_list[_main_camera_defer_lighting_subpass]->setShader(SubPass::VERTEX_SHADER, FULL_SCREEN_VERT);
    m_subpass_list[_main_camera_defer_lighting_subpass]->setShader(SubPass::FRAGMENT_SHADER, DEFER_LIGHTING_FRAG);

    SubPass::SubPassInitInfo skybox_pass_init_info{};
    skybox_pass_init_info.p_render_command_info  = m_p_render_command_info;
    skybox_pass_init_info.p_render_resource_info = m_p_render_resource_info;
    skybox_pass_init_info.renderpass             = m_renderpass;
    skybox_pass_init_info.subpass_index          = _main_camera_skybox_subpass;

    m_subpass_list[_main_camera_skybox_subpass] = std::make_shared<SubPass::SkyBoxPass>();
    m_subpass_list[_main_camera_skybox_subpass]->setShader(SubPass::VERTEX_SHADER, SKYBOX_VERT);
    m_subpass_list[_main_camera_skybox_subpass]->setShader(SubPass::FRAGMENT_SHADER, SKYBOX_FRAG);

    m_subpass_list[_main_camera_gbuffer_subpass]->initialize(&gbuffer_pass_init_info);
    m_subpass_list[_main_camera_defer_lighting_subpass]->initialize(&lighting_pass_init_info);
    m_subpass_list[_main_camera_skybox_subpass]->initialize(&skybox_pass_init_info);
}

void MainCameraDeferRenderPass::drawMultiThreading(uint32_t render_target_index, uint32_t command_buffer_index)
{
    VkClearValue clear_values[_main_camera_defer_attachment_count] = {};
    clear_values[_main_camera_defer_gbuffer_color_attachment].color    = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_gbuffer_normal_attachment].color   = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_gbuffer_position_attachment].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_color_attachment].color            = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_depth_attachment].depthStencil     = {1.0f, 0};

    VkRenderPassBeginInfo renderpass_begin_info{};
    renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.renderPass        = m_renderpass;
    renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[render_target_index];
    renderpass_begin_info.renderArea.offset = {0, 0};
    renderpass_begin_info.renderArea.extent = g_p_vulkan_context->_swapchain_extent;
    renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
    renderpass_begin_info.pClearValues      = clear_values;

    g_p_vulkan_context->_vkCmdBeginRenderPass(*m_p_render_command_info->p_current_command_buffer,
                                              &renderpass_begin_info,
                                              VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance_info.renderPass  = m_renderpass;
    inheritance_info.framebuffer = m_framebuffer_per_rendertarget[render_target_index];

    VkDebugUtilsLabelEXT label_info = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, "Mesh GBuffer", {1.0f, 1.0f, 1.0f, 1.0f}};
    g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer, &label_info);
    m_subpass_list[_main_camera_gbuffer_subpass]->drawMultiThreading(m_thread_pool,
                                                                     m_thread_data,
                                                                     inheritance_info,
                                                                     command_buffer_index,
                                                                     0,
                                                                     MESH_DRAW_THREAD_NUM);
    m_thread_pool.wait();

    std::vector<VkCommandBuffer> recorded_command_buffers;
    for (uint32_t i = 0; i < m_thread_data.size(); ++i)
    {
        recorded_command_buffers.push_back(m_thread_data[i].command_buffers[command_buffer_index]);
    }

    g_p_vulkan_context->_vkCmdExecuteCommands(*m_p_render_command_info->p_current_command_buffer,
                                              recorded_command_buffers.size(),
                                              recorded_command_buffers.data());
    g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);

    g_p_vulkan_context->_vkCmdNextSubpass(*m_p_render_command_info->p_current_command_buffer,
                                          VK_SUBPASS_CONTENTS_INLINE);
    m_subpass_list[_main_camera_defer_lighting_subpass]->draw();
    g_p_vulkan_context->_vkCmdNextSubpass(*m_p_render_command_info->p_current_command_buffer,
                                          VK_SUBPASS_CONTENTS_INLINE);
    m_subpass_list[_main_camera_skybox_subpass]->draw();
    g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
}

void MainCameraDeferRenderPass::draw(uint32_t render_target_index)
{
    VkClearValue clear_values[_main_camera_defer_attachment_count] = {};
    clear_values[_main_camera_defer_gbuffer_color_attachment].color    = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_gbuffer_normal_attachment].color   = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_gbuffer_position_attachment].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_color_attachment].color            = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_defer_depth_attachment].depthStencil     = {1.0f, 0};

    VkRenderPassBeginInfo renderpass_begin_info{};
    renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.renderPass        = m_renderpass;
    renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[render_target_index];
    renderpass_begin_info.renderArea.offset = {0, 0};
    renderpass_begin_info.renderArea.extent = g_p_vulkan_context->_swapchain_extent;
    renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
    renderpass_begin_info.pClearValues      = clear_values;

    g_p_vulkan_context->_vkCmdBeginRenderPass(*m_p_render_command_info->p_current_command_buffer,
                                              &renderpass_begin_info,
                                              VK_SUBPASS_CONTENTS_INLINE);

    m_subpass_list[_main_camera_gbuffer_subpass]->draw();
    g_p_vulkan_context->_vkCmdNextSubpass(*m_p_render_command_info->p_current_command_buffer,
                                          VK_SUBPASS_CONTENTS_INLINE);
    m_subpass_list[_main_camera_defer_lighting_subpass]->draw();
    g_p_vulkan_context->_vkCmdNextSubpass(*m_p_render_command_info->p_current_command_buffer,
                                          VK_SUBPASS_CONTENTS_INLINE);
    m_subpass_list[_main_camera_skybox_subpass]->draw();
    g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
}

void MainCameraDeferRenderPass::updateAfterSwapchainRecreate()
{
    m_thread_pool.wait();
    for (int i = 0; i < _main_camera_defer_attachment_count - 2; ++i)
    {
        if (m_renderpass_attachments[i].image != VK_NULL_HANDLE)
        {
            m_renderpass_attachments[i].destroy();
        }
    }
    for (int i = 0; i < m_framebuffer_per_rendertarget.size(); ++i)
    {
        vkDestroyFramebuffer(g_p_vulkan_context->_device, m_framebuffer_per_rendertarget[i], nullptr);
    }
    vkDestroyRenderPass(g_p_vulkan_context->_device, m_renderpass, nullptr);

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();

    for (int i = 0; i < m_subpass_list.size(); ++i)
    {
        m_subpass_list[i]->updateAfterSwapchainRecreate();
    }
}


