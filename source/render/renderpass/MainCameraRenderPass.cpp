//
// Created by kyrosz7u on 2023/5/22.
//


#include "graphic/vulkan/vulkan_util.h"
#include "render/renderpass/MainCameraRenderPass.h"


void MainCameraRenderPass::setupPipelineAttachments()
{
    m_pipeline_attachments.resize(_main_camera_framebuffer_attachment_count);

    // m_pipeline_attachments[_main_camera_framebuffer_attachment_color].format = VK_FORMAT_R8G8B8A8_UNORM;
    m_pipeline_attachments[_main_camera_framebuffer_attachment_depth].format = m_p_vulkan_context->findDepthFormat();

    for (int i = 1; i < _main_camera_framebuffer_attachment_count; ++i)
        {
            VulkanUtil::createImage(m_p_vulkan_context->_physical_device,
                                     m_p_vulkan_context->_device,
                                     m_p_vulkan_context->_swapchain_extent.width,
                                     m_p_vulkan_context->_swapchain_extent.height,
                                     m_pipeline_attachments.attachments[i].format,
                                     VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     m_pipeline_attachments.attachments[i].image,
                                     m_pipeline_attachments.attachments[i].mem,
                                     0,
                                     1,
                                     1);

            m_pipeline_attachments.attachments[i].view = VulkanUtil::createImageView(m_p_vulkan_context->_device,
                                                                            m_pipeline_attachments.attachments[i].image,
                                                                            m_pipeline_attachments.attachments[i].format,
                                                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                                                            VK_IMAGE_VIEW_TYPE_2D,
                                                                            1,
                                                                            1);
        }
}

void MainCameraRenderPass::setupFrameBuffer(std::vector<ImageAttachment> renderTargets)
{
    int targetCount = renderTargets.size();

    m_framebuffer_per_rendertarget.resize(targetCount);

    // create frame buffer for every imageview
    for (size_t i = 0; i < targetCount; i++)
    {
        VkImageView framebuffer_attachments_for_image_view[_main_camera_pass_attachment_count] = {
            _framebuffer.attachments[_main_camera_pass_gbuffer_a].view,
            _framebuffer.attachments[_main_camera_pass_gbuffer_b].view,
            _framebuffer.attachments[_main_camera_pass_gbuffer_c].view,
            _framebuffer.attachments[_main_camera_pass_backup_buffer_odd].view,
            _framebuffer.attachments[_main_camera_pass_backup_buffer_even].view,
            m_p_vulkan_context->_depth_image_view,
            m_p_vulkan_context->_swapchain_imageviews[i]};

        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags      = 0U;
        framebuffer_create_info.renderPass = _framebuffer.render_pass;
        framebuffer_create_info.attachmentCount =
            (sizeof(framebuffer_attachments_for_image_view) / sizeof(framebuffer_attachments_for_image_view[0]));
        framebuffer_create_info.pAttachments = framebuffer_attachments_for_image_view;
        framebuffer_create_info.width        = m_p_vulkan_context->_swapchain_extent.width;
        framebuffer_create_info.height       = m_p_vulkan_context->_swapchain_extent.height;
        framebuffer_create_info.layers       = 1;

        if (vkCreateFramebuffer(
                m_p_vulkan_context->_device, &framebuffer_create_info, nullptr, &m_swapchain_framebuffers[i]) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create main camera framebuffer");
        }
    }
}


