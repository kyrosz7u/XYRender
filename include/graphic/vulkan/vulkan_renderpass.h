//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_RENDERPASS_H
#define VULKANRENDER_VULKAN_RENDERPASS_H

#include "vulkan_subpass.h"

namespace VulkanAPI
{
    class RenderPassInitInfo{};

    class RenderPassBase
    {
    public:
        RenderPassBase()
        {
            assert(m_p_vulkan_context!= nullptr);
        }
        static void setVulkanContext(std::shared_ptr<VulkanContext> vulkanContext)
        {
            m_p_vulkan_context = vulkanContext;
        }
        virtual void initialize(RenderPassInitInfo* renderPassInitInfo) = 0;
        virtual void draw() = 0;
    private:
        static std::shared_ptr<VulkanContext> m_p_vulkan_context;

        std::vector<std::shared_ptr<VulkanSubPassBase>> m_subPass_list;
    };
}

#endif //VULKANRENDER_VULKAN_RENDERPASS_H
