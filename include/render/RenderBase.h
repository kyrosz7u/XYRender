
#include <memory>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_manager.h"
#include "graphic/vulkan/vulkan_renderpass.h"

using namespace VulkanAPI;

class RenderBases
{
public:
    
public:
    static void initialize(GLFWwindow* window);
    void renderFrame();
    void terminate();
private:
    static std::shared_ptr<VulkanContext> m_vulkan_context;
    static std::unique_ptr<VulkanManager> m_vulkan_manager;
};
