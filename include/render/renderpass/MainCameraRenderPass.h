//
// Created by kyrosz7u on 2023/5/22.
//

#ifndef XEXAMPLE_MAINCAMERASUBPASS_H
#define XEXAMPLE_MAINCAMERASUBPASS_H

#include "graphic/vulkan/vulkan_renderpass.h"

using namespace VulkanAPI;

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

class MainCameraRenderPass : public RenderPassBase
{
public:
    MainCameraRenderPass()
    {
        m_subpass_list.resize(_main_camera_subpass_count);
        
    }
    void initialize(RenderPassInitInfo* renderpass_init_info) override;
    void setupRenderPass();
    void setupRenderpassAttachments();
    void setupFrameBuffer(std::vector<ImageAttachment> renderTargets);
    void setupSubpass() override;
    void draw(int render_target_index) override;

private:
    VkRenderPass m_vk_renderpass;
    std::vector<ImageAttachment> m_renderpass_attachments;
    std::vector<Framebuffer> m_framebuffer_per_rendertarget;
};


#endif //XEXAMPLE_MAINCAMERASUBPASS_H
