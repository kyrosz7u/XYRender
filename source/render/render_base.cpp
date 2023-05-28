#include "render/render_base.h"

namespace RenderSystem
{
    std::shared_ptr<VulkanContext> g_p_vulkan_context = nullptr;
    RenderCommandInfo              g_render_command_info;

    // 初始化渲染器全局变量
    void RenderBase::setupGlobally(GLFWwindow* window)
    {
        g_p_vulkan_context = std::make_shared<VulkanContext>();
        g_p_vulkan_context->initialize(window);
    }
}






