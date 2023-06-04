#include <GLFW/glfw3.h>
#include "core/graphic/vulkan/vulkan_context.h"
#include "render/common_define.h"
#include "render/renderpass/renderpass_base.h"
#include "render/subpass/subpass_base.h"
#include "render/resource/render_resource.h"
#include "render/resource/render_mesh.h"
#include "render/resource/render_ubo.h"
#include <memory>

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

        virtual void Tick()
        {
            if(m_frame_count == 0)
                m_last_frame_time = glfwGetTimerValue();

            m_frame_count++;
            m_current_frame_time = glfwGetTimerValue();
            m_frame_time = (float) (m_current_frame_time - m_last_frame_time) / glfwGetTimerFrequency();
            m_last_frame_time = m_current_frame_time;
        }

        uint64_t getFrameCount() const
        {
            return m_frame_count;
        }

        float getFrameTime() const
        {
            return m_frame_time;
        }

    protected:
        RenderGlobalResourceInfo     m_render_resource_info;
        VulkanAPI::RenderCommandInfo m_render_command_info;
        uint32_t                     m_frame_count{0};
        float                        m_frame_time{0};
    private:
        uint64_t m_last_frame_time{0};
        uint64_t m_current_frame_time{0};
    };
}

