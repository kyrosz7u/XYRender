//
// Created by kyrosz7u on 2023/5/22.
//


#include "graphic/vulkan/vulkan_utils.h"
#include "render/subpass/mesh_pass.h"
#include "render/renderpass/main_camera_renderpass.h"
#include "mesh_vert.h"
#include "mesh_frag.h"

using namespace RenderSystem;

void MainCameraRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    auto main_camera_renderpass_init_info = static_cast<MainCameraRenderPassInitInfo *>(renderpass_init_info);
    m_p_render_command_info  = main_camera_renderpass_init_info->render_command_info;
    m_p_render_resource_info = main_camera_renderpass_init_info->render_resource_info;
    m_p_render_targets       = main_camera_renderpass_init_info->render_targets;

    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
}

void MainCameraRenderPass::setupRenderpassAttachments()
{
    assert(m_p_render_targets != nullptr);
    assert(m_p_render_targets->size() > 0);

    m_renderpass_attachments[_main_camera_framebuffer_attachment_color].format = (*m_p_render_targets)[0].format;
    m_renderpass_attachments[_main_camera_framebuffer_attachment_color].layout = (*m_p_render_targets)[0].layout;

    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format = g_p_vulkan_context->findDepthFormat();
    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VulkanUtil::createImage(g_p_vulkan_context,
                            g_p_vulkan_context->_swapchain_extent.width,
                            g_p_vulkan_context->_swapchain_extent.height,
                            m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].image,
                            m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].mem,
                            0,
                            1,
                            1);

    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].view =
            VulkanUtil::createImageView(g_p_vulkan_context,
                                        m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].image,
                                        m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format,
                                        VK_IMAGE_ASPECT_DEPTH_BIT,
                                        VK_IMAGE_VIEW_TYPE_2D,
                                        1,
                                        1);

}

void MainCameraRenderPass::setupRenderPass()
{
    VkAttachmentDescription attachments[_main_camera_framebuffer_attachment_count] = {};

    VkAttachmentDescription &framebuffer_image_attachment_description = attachments[_main_camera_framebuffer_attachment_color];
    framebuffer_image_attachment_description.format         = m_renderpass_attachments[_main_camera_framebuffer_attachment_color].format;
    framebuffer_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    framebuffer_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    framebuffer_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    framebuffer_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    framebuffer_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // renderpass初始化和结束时图像的布局
    framebuffer_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    framebuffer_image_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_framebuffer_attachment_color].layout;

    VkAttachmentDescription &depth_attachment_description = attachments[_main_camera_framebuffer_attachment_depth];
    depth_attachment_description.format         = m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format;
    depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].layout;

    VkSubpassDescription subpasses[_main_camera_subpass_count] = {};

    VkAttachmentReference mesh_pass_color_attachments_reference[_main_camera_subpass_count] = {};
    mesh_pass_color_attachments_reference[_main_camera_subpass_mesh].attachment =
            &framebuffer_image_attachment_description - attachments;
    // 指定在subpass执行时，管线访问图像的布局
    mesh_pass_color_attachments_reference[_main_camera_subpass_mesh].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentReference mesh_pass_depth_attachment_reference{};
    mesh_pass_depth_attachment_reference.attachment = &depth_attachment_description - attachments;
    mesh_pass_depth_attachment_reference.layout     = depth_attachment_description.finalLayout;

    VkSubpassDescription &base_pass = subpasses[_main_camera_subpass_mesh];
    base_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    base_pass.colorAttachmentCount =
            sizeof(mesh_pass_color_attachments_reference) / sizeof(mesh_pass_color_attachments_reference[0]);
    base_pass.pColorAttachments       = &mesh_pass_color_attachments_reference[0];
    base_pass.pDepthStencilAttachment = &mesh_pass_depth_attachment_reference;
    base_pass.preserveAttachmentCount = 0;
    base_pass.pPreserveAttachments    = NULL;

    VkRenderPassCreateInfo renderpass_create_info{};
    renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
    renderpass_create_info.pAttachments    = attachments;
    renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
    renderpass_create_info.pSubpasses      = subpasses;
    renderpass_create_info.dependencyCount = 0;
    renderpass_create_info.pDependencies   = nullptr;

    if (vkCreateRenderPass(
            g_p_vulkan_context->_device, &renderpass_create_info,
            nullptr, &m_renderpass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create main camera render pass");
    }
}

void MainCameraRenderPass::setupFrameBuffer()
{
    int targetCount = (*m_p_render_targets).size();

    assert(targetCount > 0);

    m_framebuffer_per_rendertarget.resize(targetCount);

    // create frame buffer for every imageview
    for (size_t i = 0; i < targetCount; i++)
    {
        std::vector<VkImageView> framebuffer_attachments;
        framebuffer_attachments.resize(_main_camera_framebuffer_attachment_count);

        framebuffer_attachments[_main_camera_framebuffer_attachment_color] = (*m_p_render_targets)[i].view;
        framebuffer_attachments[_main_camera_framebuffer_attachment_depth] =
                m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].view;

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

void MainCameraRenderPass::setupSubpass()
{
    SubPass::SubPassInitInfo mesh_pass_init_info{};
    mesh_pass_init_info.render_command_info  = m_p_render_command_info;
    mesh_pass_init_info.render_resource_info = m_p_render_resource_info;
    mesh_pass_init_info.renderpass           = m_renderpass;
    mesh_pass_init_info.subpass_index        = _main_camera_subpass_mesh;

    m_subpass_list[_main_camera_subpass_mesh] = std::make_shared<SubPass::MeshPass>();

    m_subpass_list[_main_camera_subpass_mesh]->setShader(SubPass::VERTEX_SHADER, MESH_VERT);
    m_subpass_list[_main_camera_subpass_mesh]->setShader(SubPass::FRAGMENT_SHADER, MESH_FRAG);

    m_subpass_list[_main_camera_subpass_mesh]->initialize(&mesh_pass_init_info);
}

void MainCameraRenderPass::draw(int render_target_index)
{
    VkClearValue clear_values[_main_camera_framebuffer_attachment_count] = {};
    clear_values[_main_camera_framebuffer_attachment_color].color        = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_framebuffer_attachment_depth].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderpass_begin_info{};
    renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.renderPass        = m_renderpass;
    renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[render_target_index].framebuffer;
    renderpass_begin_info.renderArea.offset = {0, 0};
    renderpass_begin_info.renderArea.extent = g_p_vulkan_context->_swapchain_extent;
    renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
    renderpass_begin_info.pClearValues      = clear_values;

    vkCmdBeginRenderPass(*m_p_render_command_info->p_current_command_buffer, &renderpass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    m_subpass_list[_main_camera_subpass_mesh]->draw();

    vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
}

void MainCameraRenderPass::updateAfterSwapchainRecreate()
{
    for (int i = 0; i < _main_camera_framebuffer_attachment_count; ++i)
    {
        if (m_renderpass_attachments[i].image != VK_NULL_HANDLE)
        {
            m_renderpass_attachments[i].destroy();
        }
    }
    for (int i = 0; i < m_framebuffer_per_rendertarget.size(); ++i)
    {
        vkDestroyFramebuffer(g_p_vulkan_context->_device, m_framebuffer_per_rendertarget[i].framebuffer, nullptr);
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


