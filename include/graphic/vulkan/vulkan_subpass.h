//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_SUBPASS_H
#define VULKANRENDER_VULKAN_SUBPASS_H

#include <vulkan/vulkan.h>
#include "vulkan_context.h"
#include "vulkan_utils.h"

#include <vector>
#include <map>

namespace VulkanAPI
{
    enum ShaderType
    {
        VERTEX_SHADER = 0,
        TESS_CONTROL_SHADER,
        TESS_EVALUATION_SHADER,
        GEOMETRY_SHADER,
        FRAGMENT_SHADER,
        COMPUTE_SHADER,
        ShaderTypeCount
    };

    struct Descriptor
    {
        VkDescriptorSetLayout layout;
        VkDescriptorSet       descriptor_set;
    };

    struct SubPassInitInfo
    {
        RenderCommandInfo* render_command_info;
        uint32_t subpass_index;
        VkRenderPass renderpass;
    };

    class VulkanSubPassBase
    {
    public:
        std::string name="dummy_subpass";
        VulkanSubPassBase()
        {
            assert(m_p_vulkan_context!= nullptr);
            m_shader_list.resize(ShaderTypeCount, std::vector<unsigned char>(0));
        }
        static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
        {
            m_p_vulkan_context = vulkanContext;
        }

        virtual void initialize(SubPassInitInfo* subPassInitInfo) = 0;
        virtual void draw() = 0;
        void setShader(ShaderType type, std::vector<unsigned char> shader)
        {
            m_shader_list[type] = shader;
        }

    protected:
        virtual void setupDescriptorSetLayout() = 0;
        virtual void setupPipelines() = 0;
        virtual void setupDescriptorSet() = 0;

        static std::shared_ptr<VulkanContext> m_p_vulkan_context;
        RenderCommandInfo* m_p_render_command_info;

        VkRenderPass renderpass;
        uint32_t subpass_index;
        VkPipelineLayout pipeline_layout;
        VkPipeline       pipeline;
        std::vector<Descriptor> m_descriptor_list;
        std::vector<std::vector<unsigned char>> m_shader_list;
    };
}

#endif //VULKANRENDER_VULKAN_SUBPASS_H
