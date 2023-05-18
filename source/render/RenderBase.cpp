#include "render/RenderBase.h"

//std::unique_ptr<VulkanContext> RenderBases::m_vulkan_context = std::make_unique<VulkanContext>();

void RenderBases::initialize(GLFWwindow* window)
{
    m_vulkan_context = std::make_shared<VulkanContext>();
    m_vulkan_context->initialize(window);
    m_vulkan_manager = std::make_unique<VulkanManager>();
    m_vulkan_manager->initialize(m_vulkan_context);
}

