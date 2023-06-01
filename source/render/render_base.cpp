#include "render/render_base.h"
#include "render/common_define.h"

namespace RenderSystem
{
    std::shared_ptr<VulkanContext> g_p_vulkan_context = nullptr;
    VulkanAPI::RenderCommandInfo   g_render_command_info;

    // 初始化渲染器全局变量
    void RenderBase::setupGlobally(GLFWwindow *window)
    {
        g_p_vulkan_context = std::make_shared<VulkanContext>();
        g_p_vulkan_context->initialize(window);
    }
}






