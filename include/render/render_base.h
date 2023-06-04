#include <memory>
#include "core/graphic/vulkan/vulkan_context.h"
#include "render/common_define.h"
#include "render/renderpass/renderpass_base.h"
#include "render/subpass/subpass_base.h"
#include "render/resource/render_resource.h"
#include "render/resource/render_mesh.h"
#include "render/resource/render_ubo.h"

#pragma once

namespace RenderSystem
{
    using namespace VulkanAPI;
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
        RenderGlobalResourceInfo     m_render_resource_info;
        VulkanAPI::RenderCommandInfo m_render_command_info;
    };
}

