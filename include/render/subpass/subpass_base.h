//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_SUBPASS_H
#define VULKANRENDER_VULKAN_SUBPASS_H

#include <vulkan/vulkan.h>
#include "core/graphic/vulkan/vulkan_context.h"
#include "core/graphic/vulkan/vulkan_utils.h"
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
            RenderCommandInfo        *p_render_command_info;
            RenderGlobalResourceInfo *p_render_resource_info;
            uint32_t                 subpass_index;
            VkRenderPass             renderpass;
        };

        class SubPassBase
        {
        public:
            std::string name = "dummy_subpass";

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
                if (pipeline != VK_NULL_HANDLE)
                {
                    vkDestroyPipeline(g_p_vulkan_context->_device, pipeline, nullptr);
                    pipeline = VK_NULL_HANDLE;
                    setupPipelines();
                }
            }

            virtual void initialize(SubPassInitInfo *subPassInitInfo) = 0;

            virtual void draw() = 0;

            virtual void drawMultiThreading(std::vector<RenderThreadData> &thread_data, VkCommandBufferInheritanceInfo& inheritance_info,
                                            uint32_t command_buffer_index, uint32_t thread_start_index, uint32_t thread_count)
            {
                throw std::runtime_error("drawMultiThreading not implemented");
            }

            virtual void updateAfterSwapchainRecreate()
            {}

        protected:
            virtual void setupDescriptorSetLayout()
            {}

            virtual void setupDescriptorSet()
            {}

            virtual void setupPipelines()
            {}

            RenderCommandInfo        *m_p_render_command_info  = nullptr;
            RenderGlobalResourceInfo *m_p_render_resource_info = nullptr;

            VkRenderPass                            m_renderpass    = VK_NULL_HANDLE;
            uint32_t                                m_subpass_index = VK_SUBPASS_EXTERNAL;
            VkPipelineLayout                        pipeline_layout = VK_NULL_HANDLE;
            VkPipeline                              pipeline        = VK_NULL_HANDLE;
//            std::vector<DescriptorSet>              m_descriptorset_list;
            std::vector<VkDescriptorSetLayout>      m_descriptor_set_layouts;
            std::vector<std::vector<unsigned char>> m_shader_list;
        };
    }
}

#endif //VULKANRENDER_VULKAN_SUBPASS_H
