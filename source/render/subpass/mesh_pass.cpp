//
// Created by kyros on 5/21/23.
//

#include "render/subpass/mesh_pass.h"
#include "render/resource/render_mesh.h"
#include "logger/logger_macros.h"

using namespace VulkanAPI;
using namespace RenderSystem;
using namespace RenderSystem::SubPass;

void MeshPass::initialize(SubPassInitInfo *subPassInitInfo)
{
    auto mesh_pass_init_info = static_cast<MeshPassInitInfo *>(subPassInitInfo);
    m_p_render_command_info  = mesh_pass_init_info->render_command_info;
    m_p_render_resource_info = mesh_pass_init_info->render_resource_info;
    subpass_index            = mesh_pass_init_info->subpass_index;
    renderpass               = mesh_pass_init_info->renderpass;

    setupDescriptorSetLayout();
    setupDescriptorSet();
    updateDescriptorSet();
    setupPipelines();
}

void MeshPass::setupDescriptorSetLayout()
{
    m_descriptorset_list.resize(1);

    DescriptorSet &descriptor_set = m_descriptorset_list[0];

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    setLayoutBindings.resize(2);

    VkDescriptorSetLayoutBinding& perframe_buffer_binding = setLayoutBindings[0];

    perframe_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perframe_buffer_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    perframe_buffer_binding.binding         = 0;
    perframe_buffer_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding& perobject_buffer_binding = setLayoutBindings[1];

    perobject_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perobject_buffer_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    perobject_buffer_binding.binding         = 1;
    perobject_buffer_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags        = 0;
    descriptorSetLayoutCreateInfo.pNext        = nullptr;
    descriptorSetLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    descriptorSetLayoutCreateInfo.pBindings    = setLayoutBindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &descriptorSetLayoutCreateInfo,
                                                nullptr,
                                                &descriptor_set.layout));
}

void MeshPass::setupDescriptorSet()
{
    for (int i = 0; i < m_descriptorset_list.size(); ++i)
    {
        VkDescriptorSetAllocateInfo allocInfo{};

        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = *m_p_render_command_info->p_descriptor_pool;
        // determines the number of descriptor sets to be allocated from the pool.
        allocInfo.descriptorSetCount = 1;
        // 每个set的布局
        allocInfo.pSetLayouts        = &m_descriptorset_list[i].layout;

        if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                     &allocInfo,
                                     &m_descriptorset_list[i].descriptor_set) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }
}

void MeshPass::updateDescriptorSet()
{
    VkDescriptorSet descriptor_set = m_descriptorset_list[0].descriptor_set;

    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.resize(2);

    VkWriteDescriptorSet& perframe_buffer_write = write_descriptor_sets[0];
    perframe_buffer_write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perframe_buffer_write.dstSet               = descriptor_set;
    perframe_buffer_write.dstBinding           = 0;
    perframe_buffer_write.dstArrayElement      = 0;
    perframe_buffer_write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perframe_buffer_write.descriptorCount      = 1;
    perframe_buffer_write.pBufferInfo          = &m_p_render_resource_info->p_render_per_frame_ubo->buffer_descriptor;

    VkWriteDescriptorSet& perobject_buffer_write = write_descriptor_sets[1];
    perobject_buffer_write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perobject_buffer_write.dstSet               = descriptor_set;
    perobject_buffer_write.dstBinding           = 1;
    perobject_buffer_write.dstArrayElement      = 0;
    perobject_buffer_write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perobject_buffer_write.descriptorCount      = 1;
    perobject_buffer_write.pBufferInfo          = &m_p_render_resource_info->p_render_model_ubo_list->buffer_descriptor;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           write_descriptor_sets.size(),
                           write_descriptor_sets.data(),
                           0,
                           nullptr);

}

void MeshPass::setupPipelines()
{
    std::vector<VkDescriptorSetLayout> descriptorset_layouts;

    for (auto &descriptor: m_descriptorset_list)
    {
        descriptorset_layouts.push_back(descriptor.layout);
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

    auto vertex_input_binding_description   = VulkanMeshVertex::getVertexInputBindingDescription();
    auto vertex_input_attribute_description = VulkanMeshVertex::getVertexInputAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = vertex_input_binding_description.size();
    vertex_input_state_create_info.pVertexBindingDescriptions      = vertex_input_binding_description.data();
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_description.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_description.data();

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
    pipelineInfo.renderPass          = renderpass;
    pipelineInfo.subpass             = subpass_index;
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

void MeshPass::draw()
{
    g_p_vulkan_context->_vkCmdBindPipeline(*m_p_render_command_info->p_current_command_buffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    g_p_vulkan_context->_vkCmdSetViewport(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                          m_p_render_command_info->p_viewport);
    g_p_vulkan_context->_vkCmdSetScissor(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                         m_p_render_command_info->p_scissor);
    // m_p_vulkan_context->_vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
    //                                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                                                 pipeline_layout,
    //                                                 0,
    //                                                 1,
    //                                                 NULL,
    //                                                 0,
    //                                                 NULL);

    for(uint32_t i=0;i<(*m_p_render_resource_info->p_visible_meshes).size();i++)
    {
        auto mesh = (*m_p_render_resource_info->p_visible_meshes)[i];
        auto index_num = mesh->m_indices.size();
        VkBuffer vertex_buffers[] = {mesh->mesh_vertex_position_buffer, mesh->mesh_vertex_normal_buffer, mesh->mesh_vertex_texcoord_buffer};
        VkDeviceSize offsets[]        = {0, 0, 0};
        g_p_vulkan_context->_vkCmdBindVertexBuffers(*m_p_render_command_info->p_current_command_buffer,
                               0,
                               sizeof(vertex_buffers) / sizeof(vertex_buffers[0]),
                               vertex_buffers,
                               offsets);
        g_p_vulkan_context->_vkCmdBindIndexBuffer(*m_p_render_command_info->p_current_command_buffer,
                                                  mesh->mesh_index_buffer,
                                                  0,
                                                  VK_INDEX_TYPE_UINT16);

        uint32_t dynamicOffset = mesh->m_index_in_dynamic_buffer * (*m_p_render_resource_info->p_render_model_ubo_list).dynamic_alignment;
        vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout,
                                0,
                                1,
                                &m_descriptorset_list[0].descriptor_set,
                                1,
                                &dynamicOffset);
        vkCmdDrawIndexed(*m_p_render_command_info->p_current_command_buffer, index_num, 1, 0, 0, 0);
    }


}

void MeshPass::updateAfterSwapchainRecreate()
{

}

