//
// Created by kyrosz7u on 2023/6/1.
//

#include "core/graphic/vulkan/vulkan_utils.h"
#include "render/renderpass/ui_overlay_pass.h"
#include "render/subpass/ui.h"
#include "render/subpass/combine_ui.h"
#include "full_screen_vert.h"
#include "combine_ui_frag.h"

using namespace RenderSystem;

void UIOverlayRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    auto ui_overlay_renderpass_init_info = static_cast<UIOverlayRenderPassInitInfo *>(renderpass_init_info);
    m_p_render_command_info  = ui_overlay_renderpass_init_info->render_command_info;
    m_p_render_resource_info = ui_overlay_renderpass_init_info->render_resource_info;
    m_p_render_targets       = ui_overlay_renderpass_init_info->render_targets;
    m_p_in_color_attachment = ui_overlay_renderpass_init_info->in_color_attachment;
    assert(m_p_in_color_attachment != nullptr);

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
}

void UIOverlayRenderPass::setupRenderpassAttachments()
{
    assert(m_p_render_targets != nullptr);
    assert(m_p_render_targets->size() > 0);

    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_in_color].format = (*m_p_in_color_attachment).format;
    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_in_color].layout = (*m_p_in_color_attachment).layout;

    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_out_color].format = (*m_p_render_targets)[0].format;
    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_out_color].layout = (*m_p_render_targets)[0].layout;

    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].format = VK_FORMAT_R8G8B8A8_UNORM;
    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VulkanUtil::createImage(g_p_vulkan_context,
                            g_p_vulkan_context->_swapchain_extent.width,
                            g_p_vulkan_context->_swapchain_extent.height,
                            m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].image,
                            m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].mem,
                            0,
                            1,
                            1);

    m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].view =
            VulkanUtil::createImageView(g_p_vulkan_context,
                                        m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].image,
                                        m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].format,
                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                        VK_IMAGE_VIEW_TYPE_2D,
                                        1,
                                        1);

}

void UIOverlayRenderPass::setupRenderPass()
{
    VkAttachmentDescription attachments[_ui_overlay_framebuffer_attachment_count] = {};

    VkAttachmentDescription &input_image_attachment_description = attachments[_ui_overlay_framebuffer_attachment_in_color];
    input_image_attachment_description.format         = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_in_color].format;
    input_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    input_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    input_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    input_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    input_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    input_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    input_image_attachment_description.finalLayout    = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_in_color].layout;

    VkAttachmentDescription &backup_image_attachment_description = attachments[_ui_overlay_framebuffer_attachment_backup_color];
    backup_image_attachment_description.format         = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].format;
    backup_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    backup_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    backup_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    backup_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    backup_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    backup_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    backup_image_attachment_description.finalLayout    = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].layout;

    VkAttachmentDescription &swapchain_image_attachment_description = attachments[_ui_overlay_framebuffer_attachment_out_color];
    swapchain_image_attachment_description.format         = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_out_color].format;
    swapchain_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    swapchain_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    swapchain_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    swapchain_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    swapchain_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // renderpass初始化和结束时图像的布局
    swapchain_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    swapchain_image_attachment_description.finalLayout    = m_renderpass_attachments[_ui_overlay_framebuffer_attachment_out_color].layout;

    VkSubpassDescription subpasses[_ui_overlay_subpass_count] = {};

    VkAttachmentReference ui_pass_color_attachments_reference[1] = {};
    ui_pass_color_attachments_reference[0].attachment =
            &backup_image_attachment_description - attachments;
    // 指定在subpass执行时，管线访问图像的布局
    ui_pass_color_attachments_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription &ui_pass = subpasses[_ui_overlay_subpass_ui];
    ui_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    ui_pass.colorAttachmentCount =
            sizeof(ui_pass_color_attachments_reference) / sizeof(ui_pass_color_attachments_reference[0]);
    ui_pass.pColorAttachments       = ui_pass_color_attachments_reference;
    ui_pass.preserveAttachmentCount = 0;
    ui_pass.pPreserveAttachments    = NULL;

    VkAttachmentReference combine_pass_input_attachments_reference[2] = {};
    combine_pass_input_attachments_reference[0].attachment =
            &input_image_attachment_description - attachments;
    combine_pass_input_attachments_reference[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    combine_pass_input_attachments_reference[1].attachment =
            &backup_image_attachment_description - attachments;
    combine_pass_input_attachments_reference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference combine_pass_color_attachments_reference[1] = {};
    combine_pass_color_attachments_reference[0].attachment =
            &swapchain_image_attachment_description - attachments;
    combine_pass_color_attachments_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription &combine_pass = subpasses[_ui_overlay_subpass_combine];
    combine_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    combine_pass.inputAttachmentCount =
            sizeof(combine_pass_input_attachments_reference) / sizeof(combine_pass_input_attachments_reference[0]);
    combine_pass.pInputAttachments       = combine_pass_input_attachments_reference;
    combine_pass.colorAttachmentCount =
            sizeof(combine_pass_color_attachments_reference) / sizeof(combine_pass_color_attachments_reference[0]);
    combine_pass.pColorAttachments       = combine_pass_color_attachments_reference;
    combine_pass.preserveAttachmentCount = 0;
    combine_pass.pPreserveAttachments    = NULL;

    VkSubpassDependency dependencies[2] = {};

    VkSubpassDependency& ui_pass_depend_on_post_effect = dependencies[0];
    ui_pass_depend_on_post_effect.srcSubpass = VK_SUBPASS_EXTERNAL;
    ui_pass_depend_on_post_effect.dstSubpass = _ui_overlay_subpass_ui;
    ui_pass_depend_on_post_effect.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    ui_pass_depend_on_post_effect.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    ui_pass_depend_on_post_effect.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    ui_pass_depend_on_post_effect.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    ui_pass_depend_on_post_effect.dependencyFlags = 0; // NOT BY REGION

    VkSubpassDependency& combine_pass_depend_on_ui_pass = dependencies[1];
    combine_pass_depend_on_ui_pass.srcSubpass = _ui_overlay_subpass_ui;
    combine_pass_depend_on_ui_pass.dstSubpass = _ui_overlay_subpass_combine;
    combine_pass_depend_on_ui_pass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    combine_pass_depend_on_ui_pass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    combine_pass_depend_on_ui_pass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    combine_pass_depend_on_ui_pass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    combine_pass_depend_on_ui_pass.dependencyFlags = 0; // NOT BY REGION

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

void UIOverlayRenderPass::setupFrameBuffer()
{
    int targetCount = (*m_p_render_targets).size();

    assert(targetCount > 0);

    m_framebuffer_per_rendertarget.resize(targetCount);

    // create frame buffer for every imageview
    for (size_t i = 0; i < targetCount; i++)
    {
        std::vector<VkImageView> framebuffer_attachments;
        framebuffer_attachments.resize(_ui_overlay_framebuffer_attachment_count);

        framebuffer_attachments[_ui_overlay_framebuffer_attachment_in_color] = (*m_p_in_color_attachment).view;
        framebuffer_attachments[_ui_overlay_framebuffer_attachment_backup_color] = 
            m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].view;
        framebuffer_attachments[_ui_overlay_framebuffer_attachment_out_color] = (*m_p_render_targets)[i].view;

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

void UIOverlayRenderPass::setupSubpass()
{
    m_subpass_list[_ui_overlay_subpass_ui] = std::make_shared<SubPass::UIPass>();
    m_subpass_list[_ui_overlay_subpass_combine] = std::make_shared<SubPass::CombineUIPass>();

    SubPass::UIPassInitInfo ui_pass_init_info;
    SubPass::CombineUIPassInitInfo combine_ui_pass_init_info;

    ui_pass_init_info.p_render_command_info = m_p_render_command_info;
    ui_pass_init_info.p_render_resource_info = m_p_render_resource_info;
    ui_pass_init_info.renderpass = m_renderpass;
    ui_pass_init_info.subpass_index = _ui_overlay_subpass_ui;

    combine_ui_pass_init_info.p_render_command_info = m_p_render_command_info;
    combine_ui_pass_init_info.p_render_resource_info = m_p_render_resource_info;
    combine_ui_pass_init_info.renderpass = m_renderpass;
    combine_ui_pass_init_info.subpass_index = _ui_overlay_subpass_combine;
    combine_ui_pass_init_info.p_input_color_attachment = m_p_in_color_attachment;
    combine_ui_pass_init_info.p_uipass_color_attachment = &m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color];

    m_subpass_list[_ui_overlay_subpass_combine]->setShader(SubPass::VERTEX_SHADER, FULL_SCREEN_VERT);
    m_subpass_list[_ui_overlay_subpass_combine]->setShader(SubPass::FRAGMENT_SHADER, COMBINE_UI_FRAG);
    m_subpass_list[_ui_overlay_subpass_ui]->initialize(&ui_pass_init_info);
    m_subpass_list[_ui_overlay_subpass_combine]->initialize(&combine_ui_pass_init_info);
}


void UIOverlayRenderPass::draw(int render_target_index)
{
    VkClearValue clear_values[_ui_overlay_framebuffer_attachment_count] = {};
    clear_values[_ui_overlay_framebuffer_attachment_in_color].color        = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_ui_overlay_framebuffer_attachment_backup_color].color = {0.0f, 0.0f, 0.0f, 0.0f};
    clear_values[_ui_overlay_framebuffer_attachment_out_color].color = {0.0f, 0.0f, 0.0f, 1.0f};

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

    m_subpass_list[_ui_overlay_subpass_ui]->draw();
    g_p_vulkan_context->_vkCmdNextSubpass(*m_p_render_command_info->p_current_command_buffer, VK_SUBPASS_CONTENTS_INLINE);
    m_subpass_list[_ui_overlay_subpass_combine]->draw();

    g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
}

void UIOverlayRenderPass::updateAfterSwapchainRecreate()
{
    vkDestroyImage(g_p_vulkan_context->_device, m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].image, nullptr);
    vkDestroyImageView(g_p_vulkan_context->_device, m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].view, nullptr);
    vkFreeMemory(g_p_vulkan_context->_device, m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].mem, nullptr);
    vkDestroyRenderPass(g_p_vulkan_context->_device, m_renderpass, nullptr);

    for (auto& framebuffer : m_framebuffer_per_rendertarget)
    {
        vkDestroyFramebuffer(g_p_vulkan_context->_device, framebuffer, nullptr);
    }

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();

    for(int i=0;i<m_subpass_list.size();++i)
    {
        m_subpass_list[i]->updateAfterSwapchainRecreate();
    }
}


