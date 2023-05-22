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
        m_subPass_list.resize(_main_camera_subpass_count);
        m_framebuffer.attachments.resize(_main_camera_framebuffer_attachment_count);
        
    }
    void initialize(RenderPassInitInfo* renderPassInitInfo) override;
    void setupPipelineAttachments() override;
    void setupFrameBuffer(std::vector<ImageAttachment> renderTargets) override;
    void draw() override;
private:
    std::vector<ImageAttachment> m_pipeline_attachments;
    std::vector<Framebuffer> m_framebuffer_per_rendertarget;
};


#endif //XEXAMPLE_MAINCAMERASUBPASS_H
