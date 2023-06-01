//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_SUBPASS_H
#define VULKANRENDER_VULKAN_SUBPASS_H

#include <vulkan/vulkan.h>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_utils.h"
#include "render/resource/render_resource.h"

#include <vector>
#include <map>

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;

    namespace SubPass
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

        struct DescriptorSet
        {
            VkDescriptorSetLayout layout;
            VkDescriptorSet       descriptor_set;
        };

        struct SubPassInitInfo
        {
            RenderCommandInfo* p_render_command_info;
            RenderGlobalResourceInfo* p_render_resource_info;
            uint32_t subpass_index;
            VkRenderPass renderpass;
        };

        class SubPassBase
        {
        public:
            std::string name="dummy_subpass";
            SubPassBase()
            {
                assert(g_p_vulkan_context != nullptr);
                m_shader_list.resize(ShaderTypeCount, std::vector<unsigned char>(0));
            }

            static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
            {
                g_p_vulkan_context = vulkanContext;
            }

            void setShader(ShaderType type, std::vector<unsigned char> shader)
            {
                m_shader_list[type] = shader;
            }

            virtual void initialize(SubPassInitInfo* subPassInitInfo) = 0;
            virtual void draw() = 0;
            virtual void updateAfterSwapchainRecreate() {}

        protected:
            virtual void setupDescriptorSetLayout() {}
            virtual void setupDescriptorSet() {}
            virtual void setupPipelines() {}

            RenderCommandInfo* m_p_render_command_info;
            RenderGlobalResourceInfo* m_p_render_resource_info;

            VkRenderPass m_renderpass;
            uint32_t     m_subpass_index;
            VkPipelineLayout pipeline_layout;
            VkPipeline              pipeline;
            VkDescriptorSetLayout                   descriptorset_layout;
            std::vector<DescriptorSet>              m_descriptorset_list;
            std::vector<std::vector<unsigned char>> m_shader_list;
        };
    }
}

#endif //VULKANRENDER_VULKAN_SUBPASS_H
