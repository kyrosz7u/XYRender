//
// Created by kyros on 6/25/23.
//

#include "render/subpass/defer_light.h"
#include "render/resource/render_mesh.h"
#include "core/logger/logger_macros.h"

using namespace VulkanAPI;
using namespace RenderSystem;
using namespace RenderSystem::SubPass;

void DeferLightPass::initialize(SubPassInitInfo *subPassInitInfo)
{
    auto mesh_pass_init_info = static_cast<DeferLightPassInitInfo *>(subPassInitInfo);
    m_p_render_command_info       = mesh_pass_init_info->p_render_command_info;
    m_p_render_resource_info      = mesh_pass_init_info->p_render_resource_info;
    m_subpass_index               = mesh_pass_init_info->subpass_index;
    m_renderpass                  = mesh_pass_init_info->renderpass;
    m_gbuffer_color_attachment    = mesh_pass_init_info->gbuffer_color_attachment;
    m_gbuffer_normal_attachment   = mesh_pass_init_info->gbuffer_normal_attachment;
    m_gbuffer_position_attachment = mesh_pass_init_info->gbuffer_position_attachment;

    setupPipeLineLayout();
    setupDescriptorSet();
    updateGlobalRenderDescriptorSet();
    setupPipelines();
}

void DeferLightPass::setupPipeLineLayout()
{
    auto &ubo_data_layout = m_descriptor_set_layouts[_mesh_defer_lighting_pass_ubo_data_layout];

    std::vector<VkDescriptorSetLayoutBinding> ubo_layout_bindings;
    ubo_layout_bindings.resize(3);

    VkDescriptorSetLayoutBinding &perframe_buffer_binding = ubo_layout_bindings[0];

    perframe_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perframe_buffer_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    perframe_buffer_binding.binding         = 0;
    perframe_buffer_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &directional_light_info_binding = ubo_layout_bindings[1];

    directional_light_info_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    directional_light_info_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    directional_light_info_binding.binding         = 1;
    directional_light_info_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &direction_light_projection_binding = ubo_layout_bindings[2];

    direction_light_projection_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    direction_light_projection_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    direction_light_projection_binding.binding         = 2;
    direction_light_projection_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo descriptorset_layout_ci;
    descriptorset_layout_ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorset_layout_ci.flags        = 0;
    descriptorset_layout_ci.pNext        = nullptr;
    descriptorset_layout_ci.bindingCount = ubo_layout_bindings.size();
    descriptorset_layout_ci.pBindings    = ubo_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &descriptorset_layout_ci,
                                                nullptr,
                                                &ubo_data_layout))

    auto &texture_data_layout = m_descriptor_set_layouts[_mesh_defer_lighting_pass_gbuffer_layout];

    std::vector<VkDescriptorSetLayoutBinding> texture_layout_bindings;
    texture_layout_bindings.resize(3);

    VkDescriptorSetLayoutBinding &gbuffer_color_binding = texture_layout_bindings[0];
    gbuffer_color_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_color_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_color_binding.binding         = 0;
    gbuffer_color_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &gbuffer_normal_binding = texture_layout_bindings[1];
    gbuffer_normal_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_normal_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_normal_binding.binding         = 1;
    gbuffer_normal_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &gbuffer_position_binding = texture_layout_bindings[2];
    gbuffer_position_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_position_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_position_binding.binding         = 2;
    gbuffer_position_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo texture_descriptorset_layout_ci;
    texture_descriptorset_layout_ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    texture_descriptorset_layout_ci.flags        = 0;
    texture_descriptorset_layout_ci.pNext        = nullptr;
    texture_descriptorset_layout_ci.bindingCount = texture_layout_bindings.size();
    texture_descriptorset_layout_ci.pBindings    = texture_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &texture_descriptorset_layout_ci,
                                                nullptr,
                                                &texture_data_layout))

    auto &directional_light_data_layout = m_descriptor_set_layouts[_mesh_defer_lighting_pass_directional_light_shadow_layout];

    std::vector<VkDescriptorSetLayoutBinding> directional_light_layout_bindings;
    directional_light_layout_bindings.resize(1);

    VkDescriptorSetLayoutBinding &directional_light_shadow_binding = directional_light_layout_bindings[0];
    directional_light_shadow_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    directional_light_shadow_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    directional_light_shadow_binding.binding         = 0;
    directional_light_shadow_binding.descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo directional_light_descriptorSetLayoutCreateInfo;
    directional_light_descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    directional_light_descriptorSetLayoutCreateInfo.flags        = 0;
    directional_light_descriptorSetLayoutCreateInfo.pNext        = nullptr;
    directional_light_descriptorSetLayoutCreateInfo.bindingCount = directional_light_layout_bindings.size();
    directional_light_descriptorSetLayoutCreateInfo.pBindings    = directional_light_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &directional_light_descriptorSetLayoutCreateInfo,
                                                nullptr,
                                                &directional_light_data_layout))
}

void DeferLightPass::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};

    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = *m_p_render_command_info->p_descriptor_pool;
    // determines the number of info sets to be allocated from the pool.
    allocInfo.descriptorSetCount = 1;
    // 每个set的布局
    allocInfo.pSetLayouts        = &m_descriptor_set_layouts[_mesh_defer_lighting_pass_ubo_data_layout];

    if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_scence_ubo_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate info sets!");
    }

    allocInfo.pSetLayouts = &m_descriptor_set_layouts[_mesh_defer_lighting_pass_gbuffer_layout];
    if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_gbuffer_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate info sets!");
    }
}

void DeferLightPass::updateGlobalRenderDescriptorSet()
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.resize(3);

    VkWriteDescriptorSet &perframe_buffer_write = write_descriptor_sets[0];
    perframe_buffer_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perframe_buffer_write.dstSet          = m_scence_ubo_descriptor_set;
    perframe_buffer_write.dstBinding      = 0;
    perframe_buffer_write.dstArrayElement = 0;
    perframe_buffer_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perframe_buffer_write.descriptorCount = 1;
    perframe_buffer_write.pBufferInfo     = &m_p_render_resource_info->p_render_per_frame_ubo
            ->buffer_infos[RenderPerFrameUBO::_scene_info_block];


    VkWriteDescriptorSet &directional_light_info_write = write_descriptor_sets[1];
    directional_light_info_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    directional_light_info_write.dstSet          = m_scence_ubo_descriptor_set;
    directional_light_info_write.dstBinding      = 1;
    directional_light_info_write.dstArrayElement = 0;
    directional_light_info_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    directional_light_info_write.descriptorCount = 1;
    directional_light_info_write.pBufferInfo     = &m_p_render_resource_info->p_render_per_frame_ubo
            ->buffer_infos[RenderPerFrameUBO::_light_info_block];

    VkWriteDescriptorSet &directional_light_probes_buffer_write = write_descriptor_sets[2];
    directional_light_probes_buffer_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    directional_light_probes_buffer_write.dstSet          = m_scence_ubo_descriptor_set;
    directional_light_probes_buffer_write.dstBinding      = 2;
    directional_light_probes_buffer_write.dstArrayElement = 0;
    directional_light_probes_buffer_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    directional_light_probes_buffer_write.descriptorCount = 1;
    directional_light_probes_buffer_write.pBufferInfo     = &m_p_render_resource_info->
            p_render_light_project_ubo_list->static_info;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           write_descriptor_sets.size(),
                           write_descriptor_sets.data(),
                           0,
                           nullptr);


    std::vector<VkWriteDescriptorSet> gbuffer_write_descriptor_sets;
    gbuffer_write_descriptor_sets.resize(3);

    VkDescriptorImageInfo gbuffer_color_info;
    VkDescriptorImageInfo gbuffer_normal_info;
    VkDescriptorImageInfo gbuffer_position_info;

    gbuffer_color_info.sampler     = VK_NULL_HANDLE;
    gbuffer_color_info.imageView   = m_gbuffer_color_attachment->view;
    gbuffer_color_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gbuffer_normal_info.sampler     = VK_NULL_HANDLE;
    gbuffer_normal_info.imageView   = m_gbuffer_normal_attachment->view;
    gbuffer_normal_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gbuffer_position_info.sampler     = VK_NULL_HANDLE;
    gbuffer_position_info.imageView   = m_gbuffer_position_attachment->view;
    gbuffer_position_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet &gbuffer_perframe_buffer_write = gbuffer_write_descriptor_sets[0];
    gbuffer_perframe_buffer_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_perframe_buffer_write.dstSet          = m_gbuffer_descriptor_set;
    gbuffer_perframe_buffer_write.dstBinding      = 0;
    gbuffer_perframe_buffer_write.dstArrayElement = 0;
    gbuffer_perframe_buffer_write.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_perframe_buffer_write.descriptorCount = 1;
    gbuffer_perframe_buffer_write.pImageInfo      = &gbuffer_color_info;

    VkWriteDescriptorSet &gbuffer_normal_write = gbuffer_write_descriptor_sets[1];
    gbuffer_normal_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_normal_write.dstSet          = m_gbuffer_descriptor_set;
    gbuffer_normal_write.dstBinding      = 1;
    gbuffer_normal_write.dstArrayElement = 0;
    gbuffer_normal_write.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_normal_write.descriptorCount = 1;
    gbuffer_normal_write.pImageInfo      = &gbuffer_normal_info;

    VkWriteDescriptorSet &gbuffer_position_write = gbuffer_write_descriptor_sets[2];
    gbuffer_position_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_position_write.dstSet          = m_gbuffer_descriptor_set;
    gbuffer_position_write.dstBinding      = 2;
    gbuffer_position_write.dstArrayElement = 0;
    gbuffer_position_write.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    gbuffer_position_write.descriptorCount = 1;
    gbuffer_position_write.pImageInfo      = &gbuffer_position_info;

    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           gbuffer_write_descriptor_sets.size(),
                           gbuffer_write_descriptor_sets.data(),
                           0,
                           nullptr);
}

void DeferLightPass::setupPipelines()
{
    std::vector<VkDescriptorSetLayout> descriptorset_layouts;

    for (auto &layout: m_descriptor_set_layouts)
    {
        descriptorset_layouts.push_back(layout);
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
    rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
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
    depth_stencil_create_info.depthTestEnable       = VK_FALSE;
    depth_stencil_create_info.depthWriteEnable      = VK_FALSE;
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

void DeferLightPass::draw()
{
//    updateGlobalRenderDescriptorSet();
    VkDebugUtilsLabelEXT label_info = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Mesh Defer Lighting", {1.0f, 1.0f, 1.0f, 1.0f}};
    g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer, &label_info);

    g_p_vulkan_context->_vkCmdBindPipeline(*m_p_render_command_info->p_current_command_buffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    g_p_vulkan_context->_vkCmdSetViewport(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                          m_p_render_command_info->p_viewport);
    g_p_vulkan_context->_vkCmdSetScissor(*m_p_render_command_info->p_current_command_buffer, 0, 1,
                                         m_p_render_command_info->p_scissor);

    VkDescriptorSet descriptor_sets[] = {m_scence_ubo_descriptor_set, m_gbuffer_descriptor_set};

    g_p_vulkan_context->_vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 pipeline_layout,
                                                 0,
                                                 2,
                                                 descriptor_sets,
                                                 0,
                                                 nullptr);

    //bind directional light shadow map
    if (m_p_render_resource_info->p_directional_light_shadow_map_descriptor_set != nullptr)
    {
        g_p_vulkan_context->_vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     pipeline_layout,
                                                     2,
                                                     1,
                                                     m_p_render_resource_info->p_directional_light_shadow_map_descriptor_set,
                                                     0,
                                                     nullptr);
    }

    g_p_vulkan_context->_vkCmdDraw(*m_p_render_command_info->p_current_command_buffer,
                                   3,
                                   1,
                                   0,
                                   0);
    g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);
}

void DeferLightPass::updateAfterSwapchainRecreate()
{

}


