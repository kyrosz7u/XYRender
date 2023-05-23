#include "render/RenderBase.h"

std::shared_ptr<VulkanContext> RenderBases::m_vulkan_context = nullptr;

// 初始化渲染器全局变量
void RenderBases::setupGlobally(GLFWwindow* window)
{
    m_vulkan_context = std::make_shared<VulkanContext>();
    m_vulkan_context->initialize(window);

    VulkanAPI::RenderPassBase::setVulkanContext(m_vulkan_context);
    VulkanAPI::VulkanSubPassBase::setVulkanContext(m_vulkan_context);
}



