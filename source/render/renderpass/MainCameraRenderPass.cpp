  //
  // Created by kyrosz7u on 2023/5/22.
  //


#include "graphic/vulkan/vulkan_utils.h"
#include "render/subpass/mesh.h"
#include "render/renderpass/MainCameraRenderPass.h"
#include "mesh_vert.h"
#include "mesh_frag.h"
#include <assert.h>

void MainCameraRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    m_p_render_command_info = renderpass_init_info->render_command_info;
    m_render_targets        = renderpass_init_info->render_targets;
    
    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
}

void MainCameraRenderPass::setupRenderpassAttachments()
{
    m_renderpass_attachments.resize(_main_camera_framebuffer_attachment_count);
    assert(m_render_targets[_swapchain_color_target][0].image!=VK_NULL_HANDLE);
    assert(m_render_targets[_swapchain_color_target][0].view!=VK_NULL_HANDLE);

    m_renderpass_attachments[_main_camera_framebuffer_attachment_color].format = m_render_targets[_swapchain_color_target][0].format;
    m_renderpass_attachments[_main_camera_framebuffer_attachment_color].layout = m_render_targets[_swapchain_color_target][0].layout;

    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format = m_p_vulkan_context->findDepthFormat();
    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VulkanUtil::createImage(m_p_vulkan_context->_physical_device,
                                m_p_vulkan_context->_device,
                                m_p_vulkan_context->_swapchain_extent.width,
                                m_p_vulkan_context->_swapchain_extent.height,
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

    m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].view = VulkanUtil::createImageView(m_p_vulkan_context->_device,
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

    VkAttachmentDescription& framebuffer_image_attachment_description                = attachments[_main_camera_framebuffer_attachment_color];
                             framebuffer_image_attachment_description.format         = m_renderpass_attachments[_main_camera_framebuffer_attachment_color].format;
                             framebuffer_image_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
                             framebuffer_image_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
                             framebuffer_image_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
                             framebuffer_image_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                             framebuffer_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                             framebuffer_image_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                             framebuffer_image_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_framebuffer_attachment_color].layout;

    VkAttachmentDescription& depth_attachment_description                = attachments[_main_camera_framebuffer_attachment_depth];
                             depth_attachment_description.format         = m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].format;
                             depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
                             depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
                             depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                             depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                             depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                             depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                             depth_attachment_description.finalLayout    = m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].layout;

    VkSubpassDescription subpasses[_main_camera_subpass_count] = {};

    VkAttachmentReference                 mesh_pass_color_attachments_reference[_main_camera_subpass_count] = {};
    mesh_pass_color_attachments_reference[_main_camera_subpass_mesh].attachment                             = &framebuffer_image_attachment_description - attachments;
    mesh_pass_color_attachments_reference[_main_camera_subpass_mesh].layout                                 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;                 // 指定attachment在subpass中的图像布局


    VkAttachmentReference mesh_pass_depth_attachment_reference {};
    mesh_pass_depth_attachment_reference.attachment = &depth_attachment_description - attachments;
    mesh_pass_depth_attachment_reference.layout     = depth_attachment_description.finalLayout;

    VkSubpassDescription& base_pass                      = subpasses[_main_camera_subpass_mesh];
                          base_pass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
                          base_pass.colorAttachmentCount = 
            sizeof(mesh_pass_color_attachments_reference) / sizeof(mesh_pass_color_attachments_reference[0]);
    base_pass.pColorAttachments       = &mesh_pass_color_attachments_reference[0];
    base_pass.pDepthStencilAttachment = &mesh_pass_depth_attachment_reference;
    base_pass.preserveAttachmentCount = 0;
    base_pass.pPreserveAttachments    = NULL;

    VkRenderPassCreateInfo renderpass_create_info {};
    renderpass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_create_info.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
    renderpass_create_info.pAttachments    = attachments;
    renderpass_create_info.subpassCount    = (sizeof(subpasses) / sizeof(subpasses[0]));
    renderpass_create_info.pSubpasses      = subpasses;
    renderpass_create_info.dependencyCount = 0;
    renderpass_create_info.pDependencies   = nullptr;

    if (vkCreateRenderPass(
            m_p_vulkan_context->_device, &renderpass_create_info, nullptr, &m_vk_renderpass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create main camera render pass");
    }
}

void MainCameraRenderPass::setupFrameBuffer()
{
    int targetCount = m_render_targets.size();
    int imageCount  = m_render_targets[0].size();

    assert(targetCount>0 && imageCount>0);
    assert(targetCount == _main_camera_render_target_count);

    m_framebuffer_per_rendertarget.resize(imageCount);

      // create frame buffer for every imageview
    for (size_t j = 0; j < imageCount; j++)
    {
        std::vector<VkImageView> framebuffer_attachments;
        framebuffer_attachments.resize(_main_camera_render_target_count);

        for(size_t i=0;i<targetCount;++i)
        {
            framebuffer_attachments[i] = m_render_targets[i][j].view;
        }

        framebuffer_attachments.push_back(m_renderpass_attachments[_main_camera_framebuffer_attachment_depth].view);

        
        VkFramebufferCreateInfo framebuffer_create_info {};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.flags           = 0U;
        framebuffer_create_info.renderPass      = m_vk_renderpass;
        framebuffer_create_info.attachmentCount = framebuffer_attachments.size();
        framebuffer_create_info.pAttachments    = framebuffer_attachments.data();
        framebuffer_create_info.width           = m_p_vulkan_context->_swapchain_extent.width;
        framebuffer_create_info.height          = m_p_vulkan_context->_swapchain_extent.height;
        framebuffer_create_info.layers          = 1;

        if (vkCreateFramebuffer(
                m_p_vulkan_context->_device, &framebuffer_create_info, nullptr, &m_framebuffer_per_rendertarget[j].framebuffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create main camera framebuffer");
        }
    }
}

void MainCameraRenderPass::setupSubpass()
{
    SubPassInitInfo mesh_pass_init_info {};
    mesh_pass_init_info.render_command_info = m_p_render_command_info;
    mesh_pass_init_info.renderpass          = m_vk_renderpass;
    mesh_pass_init_info.subpass_index       = _main_camera_subpass_mesh;

    m_subpass_list[_main_camera_subpass_mesh] = std::make_shared<subPass::MeshPass>();

    m_subpass_list[_main_camera_subpass_mesh]->setShader(VulkanAPI::VERTEX_SHADER, MESH_VERT);
    m_subpass_list[_main_camera_subpass_mesh]->setShader(VulkanAPI::FRAGMENT_SHADER, MESH_FRAG);

    m_subpass_list[_main_camera_subpass_mesh]->initialize(&mesh_pass_init_info);
}

void MainCameraRenderPass::draw(int render_target_index)
{
    VkClearValue clear_values[_main_camera_framebuffer_attachment_count] = {};
    clear_values[_main_camera_framebuffer_attachment_color].color        = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[_main_camera_framebuffer_attachment_depth].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderpass_begin_info {};
    renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.renderPass        = m_vk_renderpass;
    renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[render_target_index].framebuffer;
    renderpass_begin_info.renderArea.offset = {0, 0};
    renderpass_begin_info.renderArea.extent = m_p_vulkan_context->_swapchain_extent;
    renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
    renderpass_begin_info.pClearValues      = clear_values;

    vkCmdBeginRenderPass(*m_p_render_command_info->_p_current_command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    m_subpass_list[_main_camera_subpass_mesh]->draw();

    vkCmdEndRenderPass(*m_p_render_command_info->_p_current_command_buffer);

}



