//
// Created by kyrosz7u on 2023/6/1.
//

#include "render/subpass/combine_ui_pass.h"
#include "core/logger/logger_macros.h"
#include "core/logger/logger_macros.h"

using namespace RenderSystem::SubPass;

void CombineUIPass::initialize(SubPassInitInfo *subpass_init_info)
{
    auto combine_ui_pass_init_info = static_cast<CombineUIPassInitInfo *>(subpass_init_info);
    m_p_render_command_info     = combine_ui_pass_init_info->p_render_command_info;
    m_p_render_resource_info    = combine_ui_pass_init_info->p_render_resource_info;
    m_subpass_index             = combine_ui_pass_init_info->subpass_index;
    m_renderpass                = combine_ui_pass_init_info->renderpass;
    m_p_input_color_attachment  = combine_ui_pass_init_info->p_input_color_attachment;
    m_p_uipass_color_attachment = combine_ui_pass_init_info->p_uipass_color_attachment;

    setupDescriptorSetLayout();
    setupDescriptorSet();
    setupPipelines();
    updateDescriptorSets();
}

void CombineUIPass::setupDescriptorSetLayout()
{
    m_descriptor_set_layouts.resize(1);

    auto &descriptor_set_layout = m_descriptor_set_layouts[0];

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    setLayoutBindings.resize(2);

    VkDescriptorSetLayoutBinding &input_color_binding = setLayoutBindings[0];

    input_color_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    input_color_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    input_color_binding.binding         = 0;
    input_color_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &uipass_output_color_binding = setLayoutBindings[1];

    uipass_output_color_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    uipass_output_color_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    uipass_output_color_binding.binding         = 1;
    uipass_output_color_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags        = 0;
    descriptorSetLayoutCreateInfo.pNext        = nullptr;
    descriptorSetLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    descriptorSetLayoutCreateInfo.pBindings    = setLayoutBindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &descriptorSetLayoutCreateInfo,
                                                nullptr,
                                                &descriptor_set_layout));
}

void CombineUIPass::setupDescriptorSet()
{

    VkDescriptorSetAllocateInfo allocInfo{};

    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = *m_p_render_command_info->p_descriptor_pool;
    // determines the number of descriptor sets to be allocated from the pool.
    allocInfo.descriptorSetCount = 1;
    // 每个set的布局
    allocInfo.pSetLayouts        = &m_descriptor_set_layouts[0];

    VK_CHECK_RESULT(vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_combine_ui_descriptor_set))


}

void CombineUIPass::updateDescriptorSets()
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.resize(2);

    VkDescriptorImageInfo input_image_attachment_info{};
    input_image_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    input_image_attachment_info.imageView   = m_p_input_color_attachment->view;
    input_image_attachment_info.sampler     = VulkanUtil::getOrCreateNearestSampler(g_p_vulkan_context);

    VkDescriptorImageInfo uipass_image_attachment_info{};
    uipass_image_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    uipass_image_attachment_info.imageView   = m_p_uipass_color_attachment->view;
    uipass_image_attachment_info.sampler     = VulkanUtil::getOrCreateNearestSampler(g_p_vulkan_context);

    VkWriteDescriptorSet &input_image_write = write_descriptor_sets[0];
    input_image_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    input_image_write.dstSet          = m_combine_ui_descriptor_set;
    input_image_write.dstBinding      = 0;
    input_image_write.dstArrayElement = 0;
    input_image_write.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    input_image_write.descriptorCount = 1;
    input_image_write.pImageInfo      = &input_image_attachment_info;

    VkWriteDescriptorSet &uipass_image_write = write_descriptor_sets[1];
    uipass_image_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uipass_image_write.dstSet          = m_combine_ui_descriptor_set;
    uipass_image_write.dstBinding      = 1;
    uipass_image_write.dstArrayElement = 0;
    uipass_image_write.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    uipass_image_write.descriptorCount = 1;
    uipass_image_write.pImageInfo     = &uipass_image_attachment_info;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           write_descriptor_sets.size(),
                           write_descriptor_sets.data(),
                           0,
                           nullptr);

}

void CombineUIPass::setupPipelines()
{
    std::vector<VkDescriptorSetLayout> descriptorset_layouts;

    for (auto &descriptor: m_descriptor_set_layouts)
    {
        descriptorset_layouts.push_back(descriptor);
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges    = nullptr;
    pipeline_layout_create_info.setLayoutCount         = descriptorset_layouts.size();
    pipeline_layout_create_info.pSetLayouts            = descriptorset_layouts.data();

    if (vkCreatePipelineLayout(g_p_vulkan_context->_device,
                               &pipeline_layout_create_info,
                               nullptr,
                               &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("create " + name + " pipeline layout");
    }

    std::map<int, VkShaderModule> shader_modules;

    for (int i = 0; i < m_shader_list.size(); i++)
    {
        auto &shader = m_shader_list[i];
        if (shader.size() == 0)
        { continue; }

        shader_modules[i] = VulkanUtil::createShaderModule(g_p_vulkan_context->_device, shader);
    }

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;

    for (auto &shader_module: shader_modules)
    {
        VkPipelineShaderStageCreateInfo shader_stage_create_info{};
        shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.stage  = static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT
                << shader_module.first);
        shader_stage_create_info.module = shader_module.second;
        shader_stage_create_info.pName  = "main";
        shader_stage_create_infos.push_back(shader_stage_create_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions      = nullptr;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions    = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports    = m_p_render_command_info->p_viewport;
    viewport_state_create_info.scissorCount  = 1;
    viewport_state_create_info.pScissors     = m_p_render_command_info->p_scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable        = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth               = 1.0f;
    rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp          = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachments[1] = {};
    color_blend_attachments[0].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachments[0].blendEnable         = VK_FALSE;
    color_blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
    color_blend_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp       = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount =
            sizeof(color_blend_attachments) / sizeof(color_blend_attachments[0]);
    color_blend_state_create_info.pAttachments = &color_blend_attachments[0];
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
    depth_stencil_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.depthTestEnable       = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable      = VK_TRUE;
    depth_stencil_create_info.depthCompareOp        = VK_COMPARE_OP_LESS;
    depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_create_info.stencilTestEnable     = VK_FALSE;

    VkDynamicState                   dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates    = dynamic_states;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shader_stage_create_infos.size();
    pipelineInfo.pStages    = shader_stage_create_infos.data();

    pipelineInfo.pVertexInputState   = &vertex_input_state_create_info;
    pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
    pipelineInfo.pViewportState      = &viewport_state_create_info;
    pipelineInfo.pRasterizationState = &rasterization_state_create_info;
    pipelineInfo.pMultisampleState   = &multisample_state_create_info;
    pipelineInfo.pColorBlendState    = &color_blend_state_create_info;
    pipelineInfo.pDepthStencilState  = &depth_stencil_create_info;
    pipelineInfo.layout              = pipeline_layout;
    pipelineInfo.renderPass          = m_renderpass;
    pipelineInfo.subpass             = m_subpass_index;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState       = &dynamic_state_create_info;

    if (vkCreateGraphicsPipelines(g_p_vulkan_context->_device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &pipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("create " + name + " graphics pipeline");
    }

    for (auto &shader_module: shader_modules)
    {
        vkDestroyShaderModule(g_p_vulkan_context->_device, shader_module.second, nullptr);
    }
}

void CombineUIPass::draw()
{
    VkDebugUtilsLabelEXT label_info = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Combine UI", {1.0f, 1.0f, 1.0f, 1.0f}};
    g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer, &label_info);

    g_p_vulkan_context->_vkCmdBindPipeline(*m_p_render_command_info->p_current_command_buffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    g_p_vulkan_context->_vkCmdSetViewport(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                          m_p_render_command_info->p_viewport);
    g_p_vulkan_context->_vkCmdSetScissor(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                         m_p_render_command_info->p_scissor);


    vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout,
                            0,
                            1,
                            &m_combine_ui_descriptor_set,
                            0,
                            nullptr);
    vkCmdDrawIndexed(*m_p_render_command_info->p_current_command_buffer,
                     3,
                     1,
                     0,
                     0,
                     0);
    g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);
}

void CombineUIPass::updateAfterSwapchainRecreate()
{
    updateDescriptorSets();
}

