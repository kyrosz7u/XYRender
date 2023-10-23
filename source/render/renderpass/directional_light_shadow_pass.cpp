//
// Created by kyrosz7u on 2023/6/14.
//

#include "render/renderpass/directional_light_shadow_pass.h"
#include "render/subpass/directional_light_shadow.h"
#include "mesh_directional_light_shadow_vert.h"
#include "mesh_directional_light_shadow_frag.h"

using namespace RenderSystem;

void DirectionalLightShadowRenderPass::initialize(RenderPassInitInfo *renderpass_init_info)
{
    auto direction_light_shadow_renderpass_init_info = static_cast<DirectionalLightShadowRenderPassInitInfo *>(renderpass_init_info);
    m_p_render_command_info  = direction_light_shadow_renderpass_init_info->render_command_info;
    m_p_render_resource_info = direction_light_shadow_renderpass_init_info->render_resource_info;
    m_p_shadowmap_attachment = direction_light_shadow_renderpass_init_info->shadowmap_attachment;


    setupRenderpassAttachments();
    setupRenderPass();
    setupFrameBuffer();
    setupSubpass();
#ifdef MULTI_THREAD_RENDERING
    setupMultiThreading(m_p_shadowmap_attachment->layer_count * MESH_DRAW_THREAD_NUM);
#endif
}

void DirectionalLightShadowRenderPass::setupRenderpassAttachments()
{
    assert(m_p_shadowmap_attachment != nullptr);
    uint32_t direction_light_nums = m_p_shadowmap_attachment->layer_count;

    m_renderpass_attachments.resize(direction_light_nums);

    for (int i = 0; i < direction_light_nums; ++i)
    {
        m_renderpass_attachments[i] = *m_p_shadowmap_attachment;

        m_renderpass_attachments[i].layer_count = 1;
        m_renderpass_attachments[i].view        = VulkanUtil::createImageView(g_p_vulkan_context,
                                                                              m_renderpass_attachments[i].image,
                                                                              m_renderpass_attachments[i].format,
                                                                              VK_IMAGE_ASPECT_DEPTH_BIT,
                                                                              VK_IMAGE_VIEW_TYPE_2D, 1,
                                                                              i, 1);
    }
}

void DirectionalLightShadowRenderPass::setupRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    attachments.resize(_direction_light_attachment_count);

    VkAttachmentDescription &depth_attachment_description = attachments[0];
    depth_attachment_description.format         = m_renderpass_attachments[0].format;
    depth_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // renderpass初始化和结束时图像的布局
    depth_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    depth_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;


    std::vector<VkSubpassDescription> subpasses;
    subpasses.resize(_direction_light_shadow_subpass_count);

    VkAttachmentReference depth_attachment_reference{};
    depth_attachment_reference.attachment = 0;
    // 指定在subpass执行时，管线访问图像的布局
    depth_attachment_reference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
    uint32_t direction_light_nums = m_p_shadowmap_attachment->layer_count;
    assert(m_renderpass_attachments.size() == direction_light_nums);

    m_framebuffer_per_rendertarget.resize(direction_light_nums);

    for (int i = 0; i < direction_light_nums; ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = m_renderpass;
        framebuffer_create_info.attachmentCount = _direction_light_attachment_count;
        framebuffer_create_info.pAttachments    = &m_renderpass_attachments[i].view;
        framebuffer_create_info.width           = m_renderpass_attachments[i].width;
        framebuffer_create_info.height          = m_renderpass_attachments[i].height;
        framebuffer_create_info.layers          = 1;

        if (vkCreateFramebuffer(g_p_vulkan_context->_device,
                                &framebuffer_create_info,
                                nullptr,
                                &m_framebuffer_per_rendertarget[i]) != VK_SUCCESS)
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
                                                                      MESH_DIRECTIONAL_LIGHT_SHADOW_VERT);
    m_subpass_list[_direction_light_shadow_subpass_shadow]->setShader(SubPass::FRAGMENT_SHADER,
                                                                      MESH_DIRECTIONAL_LIGHT_SHADOW_FRAG);
    m_subpass_list[_direction_light_shadow_subpass_shadow]->initialize(&directinal_light_shadow_pass_init_info);
}

void DirectionalLightShadowRenderPass::drawMultiThreading(uint32_t render_target_index, uint32_t command_buffer_index)
{
    VkClearValue clear_values[_direction_light_attachment_count] = {};
    clear_values[_direction_light_attachment_depth].depthStencil = {1.0f, 0};

    uint32_t direction_light_nums = m_p_shadowmap_attachment->layer_count;
    assert(m_framebuffer_per_rendertarget.size() == direction_light_nums);

    VkExtent2D extent = {static_cast<uint32_t>(m_renderpass_attachments[0].width),
                         static_cast<uint32_t>(m_renderpass_attachments[0].height)};


    for (uint32_t i = 0; i < direction_light_nums; ++i)
    {
        VkImageSubresourceRange subresource_range{};
        subresource_range.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresource_range.baseMipLevel   = 0;
        subresource_range.levelCount     = 1;
        subresource_range.baseArrayLayer = 0;
        subresource_range.layerCount     = 1;

        // clear depth buffer
        g_p_vulkan_context->_vkCmdClearDepthStencilImage(*m_p_render_command_info->p_current_command_buffer,
                                                         m_renderpass_attachments[i].image,
                                                         VK_IMAGE_LAYOUT_GENERAL,
                                                         &clear_values[_direction_light_attachment_depth].depthStencil,
                                                         1,
                                                         &subresource_range);


        VkRenderPassBeginInfo renderpass_begin_info{};
        renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderPass        = m_renderpass;
        renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[i];
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = extent;
        renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues      = clear_values;

        const DirLightShadow &dir_light_shadow = (*m_dir_light_shadow_list)[i];

        for (int j = 0; j < dir_light_shadow.m_cascade_count; ++j)
        {

            g_p_vulkan_context->_vkCmdBeginRenderPass(*m_p_render_command_info->p_current_command_buffer,
                                                      &renderpass_begin_info,
                                                      VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            std::reinterpret_pointer_cast<SubPass::DirectionalLightShadowPass>(
                    m_subpass_list[_direction_light_shadow_subpass_shadow])->setDirectionalLightData(i, j);

            std::reinterpret_pointer_cast<SubPass::DirectionalLightShadowPass>(
                    m_subpass_list[_direction_light_shadow_subpass_shadow])->setViewPort(
                    dir_light_shadow.m_cascade_viewport[j]);

            VkCommandBufferInheritanceInfo inheritance_info{};
            inheritance_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance_info.renderPass  = m_renderpass;
            inheritance_info.framebuffer = m_framebuffer_per_rendertarget[i];

            VkDebugUtilsLabelEXT label_info = {
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, "Directional light shadow MultiThread",
                    {1.0f, 1.0f, 1.0f, 1.0f}};
            g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer,
                                                              &label_info);
            m_subpass_list[_direction_light_shadow_subpass_shadow]->drawMultiThreading(m_thread_pool,
                                                                                       m_thread_data,
                                                                                       inheritance_info,
                                                                                       command_buffer_index,
                                                                                       i * MESH_DRAW_THREAD_NUM,
                                                                                       MESH_DRAW_THREAD_NUM);

            m_thread_pool.wait();

            std::vector<VkCommandBuffer> recorded_command_buffers;
            for (uint32_t                i = 0; i < m_thread_data.size(); ++i)
            {
                recorded_command_buffers.push_back(m_thread_data[i].command_buffers[command_buffer_index]);
            }

            g_p_vulkan_context->_vkCmdExecuteCommands(*m_p_render_command_info->p_current_command_buffer,
                                                      recorded_command_buffers.size(),
                                                      recorded_command_buffers.data());
            g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);

            g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
        }
    }
}

void DirectionalLightShadowRenderPass::SetShadowData(const std::vector<DirLightShadow> &dir_light_shadow_list)
{
    m_dir_light_shadow_list = &dir_light_shadow_list;
}

void DirectionalLightShadowRenderPass::clearShadowMap(VkCommandBuffer &command_buffer, VkImage &image)
{
    VulkanUtil::transitionImageLayout(g_p_vulkan_context,
                                      image,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_GENERAL,
                                      1,
                                      1,
                                      VK_IMAGE_ASPECT_DEPTH_BIT,
                                      command_buffer);

    VkImageSubresourceRange subresource_range{};
    subresource_range.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    subresource_range.baseMipLevel   = 0;
    subresource_range.levelCount     = 1;
    subresource_range.baseArrayLayer = 0;
    subresource_range.layerCount     = 1;

    VkClearDepthStencilValue clear_value = {1.0f, 0};

    g_p_vulkan_context->_vkCmdClearDepthStencilImage(*m_p_render_command_info->p_current_command_buffer,
                                                     image,
                                                     VK_IMAGE_LAYOUT_GENERAL,
                                                     &clear_value,
                                                     1,
                                                     &subresource_range);

    VulkanUtil::transitionImageLayout(g_p_vulkan_context,
                                      image,
                                      VK_IMAGE_LAYOUT_GENERAL,
                                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                      1,
                                      1,
                                      VK_IMAGE_ASPECT_DEPTH_BIT,
                                      command_buffer);
}

void DirectionalLightShadowRenderPass::draw(uint32_t render_target_index)
{
    VkClearValue clear_values[_direction_light_attachment_count] = {};
    clear_values[_direction_light_attachment_depth].depthStencil = {1.0f, 0};

    uint32_t direction_light_nums = m_dir_light_shadow_list->size();
    assert(m_p_shadowmap_attachment->layer_count == direction_light_nums);
    assert(m_framebuffer_per_rendertarget.size() == direction_light_nums);

    VkExtent2D extent = {static_cast<uint32_t>(m_renderpass_attachments[0].width),
                         static_cast<uint32_t>(m_renderpass_attachments[0].height)};

    for (uint32_t i = 0; i < direction_light_nums; ++i)
    {
        VkImageSubresourceRange subresource_range{};
        subresource_range.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresource_range.baseMipLevel   = 0;
        subresource_range.levelCount     = 1;
        subresource_range.baseArrayLayer = 0;
        subresource_range.layerCount     = 1;

        // clear depth buffer
        clearShadowMap(*m_p_render_command_info->p_current_command_buffer, m_renderpass_attachments[i].image);

        VkRenderPassBeginInfo renderpass_begin_info{};
        renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderPass        = m_renderpass;
        renderpass_begin_info.framebuffer       = m_framebuffer_per_rendertarget[i];
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = extent;
        renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues      = clear_values;


        const DirLightShadow &dir_light_shadow = (*m_dir_light_shadow_list)[i];

        for (int j = 0; j < dir_light_shadow.m_cascade_count; ++j)
        {
            g_p_vulkan_context->_vkCmdBeginRenderPass(*m_p_render_command_info->p_current_command_buffer,
                                                      &renderpass_begin_info,
                                                      VK_SUBPASS_CONTENTS_INLINE);
            std::reinterpret_pointer_cast<SubPass::DirectionalLightShadowPass>(
                    m_subpass_list[_direction_light_shadow_subpass_shadow])->setDirectionalLightData(i, j);

            std::reinterpret_pointer_cast<SubPass::DirectionalLightShadowPass>(
                    m_subpass_list[_direction_light_shadow_subpass_shadow])->setViewPort(
                    dir_light_shadow.m_cascade_viewport[j]);

            m_subpass_list[_direction_light_shadow_subpass_shadow]->draw();
            g_p_vulkan_context->_vkCmdEndRenderPass(*m_p_render_command_info->p_current_command_buffer);
        }
    }
}

void DirectionalLightShadowRenderPass::updateAfterSwapchainRecreate()
{

}
