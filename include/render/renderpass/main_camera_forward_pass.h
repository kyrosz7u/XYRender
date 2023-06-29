//
// Created by kyrosz7u on 2023/5/22.
//

#ifndef XEXAMPLE_MAINCAMERA_FOWWARD_PASS_H
#define XEXAMPLE_MAINCAMERA_FOWWARD_PASS_H

#include "renderpass_base.h"
#include "render/resource/render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    struct MainCameraForwardRenderPassInitInfo : public RenderPassInitInfo
    {

    };

    class MainCameraForwardRenderPass : public RenderPassBase
    {
    public:
        enum _main_camera_subpass : unsigned int
        {
            _main_camera_subpass_mesh,
            _main_camera_subpass_skybox,
            _main_camera_subpass_count
        };

        enum _main_camera_framebuffer_attachment : unsigned int
        {
            _main_camera_framebuffer_attachment_color,
            _main_camera_framebuffer_attachment_depth,
            _main_camera_framebuffer_attachment_count
        };

        MainCameraForwardRenderPass()
        {
            m_subpass_list.resize(_main_camera_subpass_count);
            m_renderpass_attachments.resize(_main_camera_framebuffer_attachment_count);
        }

        ~MainCameraForwardRenderPass()
        {
            for (int i = 0; i < m_renderpass_attachments.size(); i++)
                m_renderpass_attachments[i].destroy();
            for (int i = 0; i < m_framebuffer_per_rendertarget.size(); i++)
                vkDestroyFramebuffer(g_p_vulkan_context->_device, m_framebuffer_per_rendertarget[i], nullptr);
        }

        void initialize(RenderPassInitInfo *renderpass_init_info) override;

        void draw(uint32_t render_target_index) override;

        void drawMultiThreading(uint32_t render_target_index, uint32_t command_buffer_index) override;

        void updateAfterSwapchainRecreate() override;

    private:
        void setupRenderPass();

        void setupRenderpassAttachments();

        void setupFrameBuffer();

        void setupSubpass() override;

    };
}

#endif //XEXAMPLE_MAINCAMERASUBPASS_H
