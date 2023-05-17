
#include "vulkan/vulkan_context.h"
// #include "vulkan/vulkan_manager.h"
#include "vulkan/vulkan_renderpass.h"

using namespace VulkanRender;

class Render
{
public:
    void initialize();
    void renderFrame();
    void terminate();
};
