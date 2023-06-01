//
// Created by kyrosz7u on 2023/6/1.
//

#include "graphic/vulkan/vulkan_utils.h"
#include "render/renderpass/ui_overlay_renderpass.h"

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

    VulkanUtil::createImage(g_p_vulkan_context,
                            g_p_vulkan_context->_swapchain_extent.width,
                            g_p_vulkan_context->_swapchain_extent.height,
                            m_renderpass_attachments[_ui_overlay_framebuffer_attachment_backup_color].format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
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
    input_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
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
        framebuffer_attachments[_ui_overlay_framebuffer_attachment_out_color] = (*m_p_render_targets)[0].view;

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
                &m_framebuffer_per_rendertarget[i].framebuffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create main camera framebuffer");
        }
    }
}

void UIOverlayRenderPass::setupSubpass()
{
    
}

void UIOverlayRenderPass::draw(int render_target_index)
{

}

void UIOverlayRenderPass::updateAfterSwapchainRecreate()
{

}


