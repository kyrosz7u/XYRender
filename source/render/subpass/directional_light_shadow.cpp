//
// Created by kyrosz7u on 2023/6/15.
//

#include "render/subpass/directional_light_shadow.h"
#include "render/resource/render_mesh.h"
#include "core/logger/logger_macros.h"

using namespace VulkanAPI;
using namespace RenderSystem;
using namespace RenderSystem::SubPass;

void DirectionalLightShadowPass::initialize(SubPassInitInfo *subPassInitInfo)
{
    auto mesh_pass_init_info = static_cast<DirectionalLightShadowPassInitInfo *>(subPassInitInfo);
    m_p_render_command_info  = mesh_pass_init_info->p_render_command_info;
    m_p_render_resource_info = mesh_pass_init_info->p_render_resource_info;
    m_subpass_index          = mesh_pass_init_info->subpass_index;
    m_renderpass             = mesh_pass_init_info->renderpass;

    setupPipeLineLayout();
    setupDescriptorSet();
    updateGlobalRenderDescriptorSet();
    setupPipelines();
}

void DirectionalLightShadowPass::setDirectionalLightData(uint32_t light_index, uint32_t cascade_index)
{
    m_directional_light_data = {light_index, cascade_index};
}

void DirectionalLightShadowPass::setViewPort(const Vector4 &extent)
{
    m_viewport_extent = extent;
}

void DirectionalLightShadowPass::setupPipeLineLayout()
{
    auto &ubo_data_layout = m_descriptor_set_layouts[_directional_shadow_layout];

    std::vector<VkDescriptorSetLayoutBinding> ubo_layout_bindings;
    ubo_layout_bindings.resize(2);

    VkDescriptorSetLayoutBinding &perlight_buffer_binding = ubo_layout_bindings[0];

    perlight_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perlight_buffer_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    perlight_buffer_binding.binding         = 0;
    perlight_buffer_binding.descriptorCount = 1;

    VkDescriptorSetLayoutBinding &perobject_buffer_binding = ubo_layout_bindings[1];

    perobject_buffer_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perobject_buffer_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    perobject_buffer_binding.binding         = 1;
    perobject_buffer_binding.descriptorCount = 1;


    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags        = 0;
    descriptorSetLayoutCreateInfo.pNext        = nullptr;
    descriptorSetLayoutCreateInfo.bindingCount = ubo_layout_bindings.size();
    descriptorSetLayoutCreateInfo.pBindings    = ubo_layout_bindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(g_p_vulkan_context->_device,
                                                &descriptorSetLayoutCreateInfo,
                                                nullptr,
                                                &ubo_data_layout))
}

void DirectionalLightShadowPass::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};

    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = *m_p_render_command_info->p_descriptor_pool;
    // determines the number of info sets to be allocated from the pool.
    allocInfo.descriptorSetCount = 1;
    // 每个set的布局
    allocInfo.pSetLayouts        = &m_descriptor_set_layouts[_directional_shadow_layout];

    m_p_render_resource_info->p_render_light_project_ubo_list->DescribeUpdate(this, std::bind(
            &DirectionalLightShadowPass::updateGlobalRenderDescriptorSet, this));

    if (vkAllocateDescriptorSets(g_p_vulkan_context->_device,
                                 &allocInfo,
                                 &m_dir_shadow_ubo_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate info sets!");
    }
}

void DirectionalLightShadowPass::updateGlobalRenderDescriptorSet()
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.resize(2);

    VkWriteDescriptorSet &perframe_buffer_write = write_descriptor_sets[0];
    perframe_buffer_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perframe_buffer_write.dstSet          = m_dir_shadow_ubo_descriptor_set;
    perframe_buffer_write.dstBinding      = 0;
    perframe_buffer_write.dstArrayElement = 0;
    perframe_buffer_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perframe_buffer_write.descriptorCount = 1;
    perframe_buffer_write.pBufferInfo     = &m_p_render_resource_info->p_render_light_project_ubo_list->dynamic_info;

    VkWriteDescriptorSet &perobject_buffer_write = write_descriptor_sets[1];
    perobject_buffer_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perobject_buffer_write.dstSet          = m_dir_shadow_ubo_descriptor_set;
    perobject_buffer_write.dstBinding      = 1;
    perobject_buffer_write.dstArrayElement = 0;
    perobject_buffer_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perobject_buffer_write.descriptorCount = 1;
    perobject_buffer_write.pBufferInfo     = &m_p_render_resource_info->p_render_model_ubo_list->dynamic_info;


    vkUpdateDescriptorSets(g_p_vulkan_context->_device,
                           write_descriptor_sets.size(),
                           write_descriptor_sets.data(),
                           0,
                           nullptr);

}

void DirectionalLightShadowPass::setupPipelines()
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
        throw std::runtime_error("create " + name + " m_pipeline layout");
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

    auto vertex_input_binding_description   = VulkanMeshVertex::getLightVertexInputBindingDescription();
    auto vertex_input_attribute_description = VulkanMeshVertex::getLightVertexInputAttributeDescription();

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
    rasterization_state_create_info.cullMode                = VK_CULL_MODE_NONE;
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
    depth_stencil_create_info.depthTestEnable       = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable      = VK_TRUE;
    depth_stencil_create_info.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
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
                                  &m_pipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("create " + name + " graphics m_pipeline");
    }

    for (auto &shader_module: shader_modules)
    {
        vkDestroyShaderModule(g_p_vulkan_context->_device, shader_module.second, nullptr);
    }
}

void DirectionalLightShadowPass::drawSingleThread(VkCommandBuffer &command_buffer,
                                                  VkCommandBufferInheritanceInfo &inheritance_info,
                                                  uint32_t submesh_start_index,
                                                  uint32_t submesh_end_index)
{
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &inheritance_info;

    VK_CHECK_RESULT(g_p_vulkan_context->_vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info))

    VkViewport viewport{};
    viewport.x        = m_viewport_extent.x;
    viewport.y        = m_viewport_extent.y;
    viewport.width    = m_viewport_extent.z;
    viewport.height   = m_viewport_extent.w;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(m_viewport_extent.z), static_cast<uint32_t>(m_viewport_extent.w)};

    g_p_vulkan_context->_vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    g_p_vulkan_context->_vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    g_p_vulkan_context->_vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    auto &render_submeshes = *m_p_render_resource_info->p_render_submeshes;

    for (uint32_t i = submesh_start_index; i < submesh_end_index; ++i)
    {
        const auto submesh     = (*m_p_render_resource_info->p_render_submeshes)[i];
        const auto parent_mesh = submesh.parent_mesh.lock();
        if (parent_mesh == nullptr)
        {
            continue;
        }

        VkBuffer     vertex_buffers[] = {parent_mesh->mesh_vertex_position_buffer};
        VkDeviceSize offsets[]        = {0};
        g_p_vulkan_context->_vkCmdBindVertexBuffers(command_buffer,
                                                    0,
                                                    sizeof(vertex_buffers) / sizeof(vertex_buffers[0]),
                                                    vertex_buffers,
                                                    offsets);
        g_p_vulkan_context->_vkCmdBindIndexBuffer(command_buffer,
                                                  parent_mesh->mesh_index_buffer,
                                                  0,
                                                  VK_INDEX_TYPE_UINT16);

        // bind model and light ubo
        uint32_t dynamic_offset[2];

        dynamic_offset[0] = (m_directional_light_data.light_index *
                             m_p_render_resource_info->kDirectionalLightInfo.max_cascade_count +
                             m_directional_light_data.cascade_index) *
                            (*m_p_render_resource_info->p_render_light_project_ubo_list).dynamic_alignment;

        dynamic_offset[1] = parent_mesh->m_index_in_dynamic_buffer *
                            (*m_p_render_resource_info->p_render_model_ubo_list).dynamic_alignment;

        g_p_vulkan_context->_vkCmdBindDescriptorSets(command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     pipeline_layout,
                                                     0,
                                                     1,
                                                     &m_dir_shadow_ubo_descriptor_set,
                                                     2,
                                                     dynamic_offset);
        g_p_vulkan_context->_vkCmdDrawIndexed(command_buffer,
                                              submesh.index_count,
                                              1,
                                              submesh.index_offset,
                                              submesh.vertex_offset,
                                              0);
    }

    VK_CHECK_RESULT(g_p_vulkan_context->_vkEndCommandBuffer(command_buffer))
}

void DirectionalLightShadowPass::drawMultiThreading(ThreadPool &thread_pool,
                                                    std::vector<RenderThreadData> &thread_data,
                                                    VkCommandBufferInheritanceInfo &inheritance_info,
                                                    uint32_t command_buffer_index,
                                                    uint32_t thread_start_index,
                                                    uint32_t thread_count)
{
    uint32_t submesh_count                = m_p_render_resource_info->p_render_submeshes->size();
    uint32_t submesh_per_thread           = submesh_count / thread_count;
    uint32_t submesh_per_thread_remainder = submesh_count % thread_count;

    for (uint32_t i = 0; i < thread_count; ++i)
    {
        auto     &command_buffer     = thread_data[thread_start_index + i].command_buffers[command_buffer_index];
        uint32_t submesh_start_index = i * submesh_per_thread;
        uint32_t submesh_end_index   = submesh_start_index + submesh_per_thread;
        if (i == thread_count - 1)
        {
            submesh_end_index += submesh_per_thread_remainder;
        }
        thread_pool.threads[thread_start_index + i]->addJob(
                [this, &command_buffer, &inheritance_info, submesh_start_index, submesh_end_index]()
                {
                    drawSingleThread(command_buffer,
                                     inheritance_info,
                                     submesh_start_index,
                                     submesh_end_index);
                });
    }
}

void DirectionalLightShadowPass::draw()
{
    VkDebugUtilsLabelEXT label_info = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Directional light shadow", {1.0f, 1.0f, 1.0f, 1.0f}};
    g_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer, &label_info);

    g_p_vulkan_context->_vkCmdBindPipeline(*m_p_render_command_info->p_current_command_buffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkViewport viewport{};
    viewport.x        = m_viewport_extent.x;
    viewport.y        = m_viewport_extent.y;
    viewport.width    = m_viewport_extent.z;
    viewport.height   = m_viewport_extent.w;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(m_p_render_resource_info->kDirectionalLightInfo.shadowmap_width),
                      static_cast<uint32_t>(m_p_render_resource_info->kDirectionalLightInfo.shadowmap_height)};

    g_p_vulkan_context->_vkCmdSetViewport(*m_p_render_command_info->p_current_command_buffer, 0, 1, &viewport);
    g_p_vulkan_context->_vkCmdSetScissor(*m_p_render_command_info->p_current_command_buffer, 0, 1, &scissor);


    auto &render_submeshes = *m_p_render_resource_info->p_render_submeshes;

    for (uint32_t i = 0; i < render_submeshes.size(); i++)
    {
        const auto submesh     = (*m_p_render_resource_info->p_render_submeshes)[i];
        const auto parent_mesh = submesh.parent_mesh.lock();
        if (parent_mesh == nullptr)
        {
            continue;
        }

        VkBuffer     vertex_buffers[] = {parent_mesh->mesh_vertex_position_buffer};
        VkDeviceSize offsets[]        = {0};
        g_p_vulkan_context->_vkCmdBindVertexBuffers(*m_p_render_command_info->p_current_command_buffer,
                                                    0,
                                                    sizeof(vertex_buffers) / sizeof(vertex_buffers[0]),
                                                    vertex_buffers,
                                                    offsets);
        g_p_vulkan_context->_vkCmdBindIndexBuffer(*m_p_render_command_info->p_current_command_buffer,
                                                  parent_mesh->mesh_index_buffer,
                                                  0,
                                                  VK_INDEX_TYPE_UINT16);

        // bind model and light ubo
        uint32_t dynamic_offset[2];

        uint32_t ubo_offset = m_p_render_command_info->current_command_index *
                              MAX_DIRECTIONAL_LIGHT_COUNT * MAX_SHADOWMAP_CASCADE_COUNT +
                              m_directional_light_data.light_index *
                              MAX_SHADOWMAP_CASCADE_COUNT + m_directional_light_data.cascade_index;

        ubo_offset *= (*m_p_render_resource_info->p_render_light_project_ubo_list).dynamic_alignment;

        dynamic_offset[0] = ubo_offset;

        dynamic_offset[1] = parent_mesh->m_index_in_dynamic_buffer *
                            (*m_p_render_resource_info->p_render_model_ubo_list).dynamic_alignment;

        g_p_vulkan_context->_vkCmdBindDescriptorSets(*m_p_render_command_info->p_current_command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     pipeline_layout,
                                                     0,
                                                     1,
                                                     &m_dir_shadow_ubo_descriptor_set,
                                                     2,
                                                     dynamic_offset);
        g_p_vulkan_context->_vkCmdDrawIndexed(*m_p_render_command_info->p_current_command_buffer,
                                              submesh.index_count,
                                              1,
                                              submesh.index_offset,
                                              submesh.vertex_offset,
                                              0);
    }
    g_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(*m_p_render_command_info->p_current_command_buffer);
}

void DirectionalLightShadowPass::updateAfterSwapchainRecreate()
{

}
