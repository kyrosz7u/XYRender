#include <memory>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_manager.h"
#include "graphic/vulkan/vulkan_renderpass.h"
#include "graphic/vulkan/vulkan_subpass.h"

#pragma once

using namespace VulkanAPI;

struct RenderCommandInfo
{
    uint32_t         _current_frame_index;
    uint32_t*        _p_current_frame_index;
    VkCommandBuffer  _current_command_buffer;
    VkCommandBuffer* _p_current_command_buffer;
    VkCommandPool    _current_command_pool;
    VkCommandPool*   _p_command_pools;

    VkViewport       _viewport;
    VkRect2D         _scissor;
};

class RenderBases
{
public:


public:
    static void initialize(GLFWwindow* window);

    virtual void Tick() = 0;
//    void renderFrame();
//    void terminate();
private:
    static std::shared_ptr<VulkanContext> m_vulkan_context;
    static std::unique_ptr<VulkanManager> m_vulkan_manager;
    RenderCommandInfo m_render_command_info;
};
