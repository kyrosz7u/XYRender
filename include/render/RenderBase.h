#include <memory>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_manager.h"
#include "graphic/vulkan/vulkan_renderpass.h"
#include "graphic/vulkan/vulkan_subpass.h"

#pragma once

using namespace VulkanAPI;

class RenderBases
{
public:
    RenderBases()
    {
        assert(m_vulkan_context!= nullptr);
        assert(m_vulkan_manager!= nullptr);
    }

public:
    static void initialize(GLFWwindow* window);

    virtual void Tick() = 0;
//    void renderFrame();
//    void terminate();
protected:
    static std::shared_ptr<VulkanContext> m_vulkan_context;
    static std::unique_ptr<VulkanManager> m_vulkan_manager;
    RenderCommandInfo m_render_command_info;
};
