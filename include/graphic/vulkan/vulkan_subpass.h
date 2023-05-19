//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_SUBPASS_H
#define VULKANRENDER_VULKAN_SUBPASS_H

#include <vulkan/vulkan.h>

namespace VulkanAPI
{   
    /*    
        VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
        VK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
        VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
        VK_SHADER_STAGE_COMPUTE_BIT = 0x00000020, 
     */
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

    class Subpass
    {
    public:
        Subpass()
        {
            shaderList.resize(ShaderTypeCount, VK_NULL_HANDLE);
        }
        void initialize(VkRenderPass render_pass, VkImageView input_attachment);
        void draw();
        void setShader(ShaderType type, VkShaderModule* shader)
        {
            shaderList[type] = shader;
        }
    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    private:
        VkPipelineLayout pipelineLayout;
        VkPipeline       pipeline;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet       descriptorSet;
        VkRenderPass renderPass;

        std::vector<VkShaderModule*> shaderList;
    };
}

#endif //VULKANRENDER_VULKAN_SUBPASS_H
