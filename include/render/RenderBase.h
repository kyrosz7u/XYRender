#include <memory>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_renderpass.h"
#include "graphic/vulkan/vulkan_subpass.h"

#pragma once

using namespace VulkanAPI;

class RenderBases
{
public:
    VkViewport m_viewport = {0, 0, 1280, 768, 0, 1};
    VkRect2D   m_scissor  = {{0, 0}, {1280, 768}};
public:
    RenderBases()
    {
        assert(m_vulkan_context!= nullptr);
    }

    static void setupGlobally(GLFWwindow* window);
    virtual void initialize() = 0;
    virtual void Tick() = 0;
//    void renderFrame();
//    void terminate();
protected:
    static std::shared_ptr<VulkanContext> m_vulkan_context;
    RenderCommandInfo m_render_command_info;
};
