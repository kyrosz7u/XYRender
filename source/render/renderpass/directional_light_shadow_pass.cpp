//
// Created by kyrosz7u on 2023/6/14.
//

#include "render/renderpass/directional_light_shadow_pass.h"
#include "render/subpass/directional_light_shadow.h"
#include "mesh_point_light_shadow_vert.h"
#include "mesh_point_light_shadow_frag.h"

using namespace RenderSystem;

void DirectionalLightShadowRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    auto direction_light_shadow_renderpass_init_info = static_cast<DirectionalLightShadowRenderPassInitInfo *>(renderpass_init_info);
    m_p_render_command_info  = direction_light_shadow_renderpass_init_info->render_command_info;
    m_p_render_resource_info = direction_light_shadow_renderpass_init_info->render_resource_info;
    m_p_render_targets       = direction_light_shadow_renderpass_init_info->render_targets;

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
}

void DirectionalLightShadowRenderPass::setupRenderpassAttachments()
{
    assert(m_p_render_targets != nullptr);
    assert(m_p_render_targets->size() > 0);

    int direction_light_nums = m_p_render_targets->size();

    m_renderpass_attachments.resize(direction_light_nums);

    for (int i = 0; i < direction_light_nums; ++i)
    {
        m_renderpass_attachments[i] = (*m_p_render_targets)[i];
    }

}

void DirectionalLightShadowRenderPass::setupRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    attachments.resize(_direction_light_attachment_count);

    VkAttachmentDescription &depth_attachment_description = attachments[0];
    depth_attachment_description.format         = m_renderpass_attachments[0].format;
    depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // renderpass初始化和结束时图像的布局
    depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment_description.finalLayout    = m_renderpass_attachments[0].layout;


    std::vector<VkSubpassDescription> subpasses;
    subpasses.resize(_direction_light_shadow_subpass_count);

    VkAttachmentReference depth_attachment_reference{};
    depth_attachment_reference.attachment = 0;
    // 指定在subpass执行时，管线访问图像的布局
    depth_attachment_reference.layout     = attachments[0].finalLayout;

    VkSubpassDescription &base_pass = subpasses[_direction_light_shadow_subpass_shadow];
    base_pass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    base_pass.colorAttachmentCount    = 0;
    base_pass.pColorAttachments       = nullptr;
    base_pass.pDepthStencilAttachment = &depth_attachment_reference;
    base_pass.preserveAttachmentCount = 0;
    base_pass.pPreserveAttachments    = NULL;

    std::vector<VkSubpassDependency> dependencies;
    dependencies.resize(2);

    VkSubpassDependency &base_pass_dependency = dependencies[0];
    base_pass_dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
    base_pass_dependency.dstSubpass      = _direction_light_shadow_subpass_shadow;
    base_pass_dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    base_pass_dependency.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    base_pass_dependency.srcAccessMask   = VK_ACCESS_NONE;
    base_pass_dependency.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    base_pass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDependency &other_pass_dependency = dependencies[1];
    other_pass_dependency.srcSubpass      = _direction_light_shadow_subpass_shadow;
    other_pass_dependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
    other_pass_dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    other_pass_dependency.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    other_pass_dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    other_pass_dependency.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    other_pass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderpass_create_info{};
    renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_create_info.attachmentCount = attachments.size();
    renderpass_create_info.pAttachments    = attachments.data();
    renderpass_create_info.subpassCount    = subpasses.size();
    renderpass_create_info.pSubpasses      = subpasses.data();
    renderpass_create_info.dependencyCount = dependencies.size();
    renderpass_create_info.pDependencies   = dependencies.data();

    if (vkCreateRenderPass(
            g_p_vulkan_context->_device, &renderpass_create_info,
            nullptr, &m_renderpass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create main camera render pass");
    }
}

void DirectionalLightShadowRenderPass::setupFrameBuffer()
{
    int direction_light_nums = m_p_render_targets->size();

    assert(direction_light_nums > 0);

    m_framebuffer_per_rendertarget.resize(direction_light_nums);

    for (int i = 0; i < direction_light_nums; ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = m_renderpass;
        framebuffer_create_info.attachmentCount = _direction_light_attachment_count;
        framebuffer_create_info.pAttachments    = &(*m_p_render_targets)[i].view;
        framebuffer_create_info.width           = (*m_p_render_targets)[i].width;
        framebuffer_create_info.height          = (*m_p_render_targets)[i].height;
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

void DirectionalLightShadowRenderPass::setupSubpass()
{
    SubPass::DirectionalLightShadowPassInitInfo directinal_light_shadow_pass_init_info{};
    directinal_light_shadow_pass_init_info.p_render_command_info  = m_p_render_command_info;
    directinal_light_shadow_pass_init_info.p_render_resource_info = m_p_render_resource_info;
    directinal_light_shadow_pass_init_info.renderpass             = m_renderpass;
    directinal_light_shadow_pass_init_info.subpass_index          = _direction_light_shadow_subpass_shadow;

    m_subpass_list[_direction_light_shadow_subpass_shadow] = std::make_shared<SubPass::DirectionalLightShadowPass>();
    m_subpass_list[_direction_light_shadow_subpass_shadow]->setShader(SubPass::VERTEX_SHADER,
                                                                      MESH_POINT_LIGHT_SHADOW_VERT);
    m_subpass_list[_direction_light_shadow_subpass_shadow]->setShader(SubPass::FRAGMENT_SHADER,
                                                                      MESH_POINT_LIGHT_SHADOW_FRAG);
    m_subpass_list[_direction_light_shadow_subpass_shadow]->initialize(&directinal_light_shadow_pass_init_info);

}

void DirectionalLightShadowRenderPass::draw(int render_target_index)
{
    VkClearValue clear_values[_direction_light_attachment_count] = {};
    clear_values[_direction_light_attachment_depth].depthStencil = {1.0f, 0};

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

    m_subpass_list[_direction_light_shadow_subpass_shadow]->draw();
    g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
}

void DirectionalLightShadowRenderPass::updateAfterSwapchainRecreate()
{

}
