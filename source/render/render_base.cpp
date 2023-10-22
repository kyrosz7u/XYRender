#include "render/render_base.h"
#include "render/common_define.h"

namespace RenderSystem
{
    // 渲染器全局变量定义
    std::shared_ptr<VulkanContext>            g_p_vulkan_context = nullptr;
    const uint32_t DEFAULT_THREAD_COUNT = 4;
    const uint32_t MESH_DRAW_THREAD_NUM = 4;

    // 初始化渲染器全局变量
    void RenderBase::setupGlobally(GLFWwindow *window)
    {
        g_p_vulkan_context = std::make_shared<VulkanContext>();
        g_p_vulkan_context->initialize(window);
    }
}






