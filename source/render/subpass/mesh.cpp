//
// Created by kyros on 5/21/23.
//

#include "render/subpass/mesh.h"

using namespace VulkanAPI;
using namespace subPass;

void MeshPass::initialize(SubPassInitInfo *subPassInitInfo)
{
    renderpass = subPassInitInfo->renderpass;
    subpass_index = subPassInitInfo->subpass_index;
}

void MeshPass::setupDescriptorSetLayout()
{

}

void MeshPass::updateDescriptorSet()
{

}

void MeshPass::setupPipelines()
{
    std::vector<VkDescriptorSetLayout> descriptorset_layouts;

    for (auto &descriptor: m_descriptor_list)
    {
        descriptorset_layouts.push_back(descriptor.layout);
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = descriptorset_layouts.size();
    pipeline_layout_create_info.pSetLayouts = descriptorset_layouts.data();

    if (vkCreatePipelineLayout(m_p_vulkan_context->_device,
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

        shader_modules[i] = VulkanUtil::createShaderModule(m_p_vulkan_context->_device, shader);
    }

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;

    for(auto& shader_module:shader_modules)
    {
        VkPipelineShaderStageCreateInfo shader_stage_create_info{};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT<<shader_module.first);
        shader_stage_create_info.module = shader_module.second;
        shader_stage_create_info.pName = "main";
        shader_stage_create_infos.push_back(shader_stage_create_info);
    }

//    auto vertex_binding_descriptions   = MeshVertex::getBindingDescriptions();
//    auto vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
//    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
//    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vertex_input_state_create_info.vertexBindingDescriptionCount   = vertex_binding_descriptions.size();
//    vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_binding_descriptions[0];
//    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
//    vertex_input_state_create_info.pVertexAttributeDescriptions    = &vertex_attribute_descriptions[0];

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
    input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state_create_info {};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports    = m_p_viewport;
    viewport_state_create_info.scissorCount  = 1;
    viewport_state_create_info.pScissors     = m_p_scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
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

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachments[1] = {};
    color_blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
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
    color_blend_state_create_info.pAttachments      = &color_blend_attachments[0];
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
    depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.depthTestEnable  = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable = VK_TRUE;
    depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS;
    depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_create_info.stencilTestEnable     = VK_FALSE;

    VkDynamicState                   dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
    dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates    = dynamic_states;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = shader_stage_create_infos.size();
    pipelineInfo.pStages             = shader_stage_create_infos.data();
//    pipelineInfo.pVertexInputState   = &vertex_input_state_create_info;
    pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
    pipelineInfo.pViewportState      = &viewport_state_create_info;
    pipelineInfo.pRasterizationState = &rasterization_state_create_info;
    pipelineInfo.pMultisampleState   = &multisample_state_create_info;
    pipelineInfo.pColorBlendState    = &color_blend_state_create_info;
    pipelineInfo.pDepthStencilState  = &depth_stencil_create_info;
    pipelineInfo.layout              = pipeline_layout;
    pipelineInfo.renderPass          = renderpass;
    pipelineInfo.subpass             = subpass_index;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState       = &dynamic_state_create_info;

    if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &pipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("create " + name + " graphics pipeline");
    }

    for(auto& shader_module:shader_modules)
    {
        vkDestroyShaderModule(m_p_vulkan_context->_device, shader_module.second, nullptr);
    }
}


