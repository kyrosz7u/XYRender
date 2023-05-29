#include <memory>
#include "graphic/vulkan/vulkan_context.h"
#include "render/renderpass/renderpass_base.h"
#include "render/subpass/subpass_base.h"
#include "render/resource/render_resource.h
#include "render/resource/render_mesh.h"
#include "render/resource/render_ubo.h"


#ifndef XEXAMPLE_RENDER_BASE_H
#define XEXAMPLE_RENDER_BASE_H

using namespace VulkanAPI;

namespace RenderSystem
{
    extern std::shared_ptr<VulkanContext> g_p_vulkan_context;


    class RenderBase
    {
    public:
        VkViewport m_viewport = {0, 0, 1280, 768, 0, 1};
        VkRect2D   m_scissor  = {{0,    0},
                                 {1280, 768}};
    public:
        RenderBase()
        {
            assert(g_p_vulkan_context != nullptr);
        }

        virtual ~RenderBase() = default;

        static void setupGlobally(GLFWwindow *window);

        virtual void initialize() = 0;

        virtual void Tick() = 0;

    protected:
        RenderGlobalResourceInfo render_resource_info;
        RenderCommandInfo        render_command_info;
    };
}
#endif //XEXAMPLE_RENDER_BASE_H

