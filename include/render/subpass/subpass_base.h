//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_SUBPASS_H
#define VULKANRENDER_VULKAN_SUBPASS_H

#include "vulkan/vulkan.h"
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_utils.h"
#include "render/render_base.h"

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

        struct Descriptor
        {
            VkDescriptorSetLayout layout;
            VkDescriptorSet       descriptor_set;
        };

        struct SubPassInitInfo
        {
            RenderCommandInfo* render_command_info;
            RenderGlobalResourceInfo* render_resource_info;
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
            virtual void updateAfterSwapchainRecreate() = 0;

        protected:
            virtual void setupDescriptorSetLayout() = 0;
            virtual void setupDescriptorSet() = 0;
            virtual void setupPipelines() = 0;

            RenderCommandInfo* m_p_render_command_info;
            RenderGlobalResourceInfo* m_p_render_resource_info;

            VkRenderPass renderpass;
            uint32_t subpass_index;
            VkPipelineLayout pipeline_layout;
            VkPipeline       pipeline;
            std::vector<Descriptor> m_descriptor_list;
            std::vector<std::vector<unsigned char>> m_shader_list;
        };
    }
}

#endif //VULKANRENDER_VULKAN_SUBPASS_H
