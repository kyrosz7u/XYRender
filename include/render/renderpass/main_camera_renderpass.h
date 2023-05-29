//
// Created by kyrosz7u on 2023/5/22.
//

#ifndef XEXAMPLE_MAINCAMERASUBPASS_H
#define XEXAMPLE_MAINCAMERASUBPASS_H

#include "renderpass_base.h"
#include "render/resource/render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    struct MainCameraRenderPassInitInfo:public RenderPassInitInfo
    {

    };

    class MainCameraRenderPass : public RenderPassBase
    {
    public:
        enum _main_camera_subpass
        {
            _main_camera_subpass_mesh,
            _main_camera_subpass_count
        };

        enum _main_camera_framebuffer_attachment
        {
            _main_camera_framebuffer_attachment_color,
            _main_camera_framebuffer_attachment_depth,
            _main_camera_framebuffer_attachment_count
        };
        MainCameraRenderPass()
        {
            m_subpass_list.resize(_main_camera_subpass_count);
        }
        void initialize(RenderPassInitInfo* renderpass_init_info) override;
        void draw(int render_target_index) override;
        void updateAfterSwapchainRecreate() override;
    private:
        void setupRenderPass();
        void setupRenderpassAttachments();
        void setupFrameBuffer();
        void setupSubpass() override;

    private:
        VkRenderPass m_vk_renderpass;
        std::vector<ImageAttachment> m_renderpass_attachments;
        std::vector<Framebuffer>   m_framebuffer_per_rendertarget;
    };
}

#endif //XEXAMPLE_MAINCAMERASUBPASS_H
