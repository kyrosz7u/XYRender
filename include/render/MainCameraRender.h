//
// Created by kyrosz7u on 2023/5/19.
//

#ifndef XEXAMPLE_MAINCAMERARENDER_H
#define XEXAMPLE_MAINCAMERARENDER_H

#include "RenderBase.h"
#include "render/renderpass/MainCameraRenderPass.h"

class MainCameraRender:public RenderBases
{
public:
    ~MainCameraRender() override;
    void initialize() override;
    void setupRenderTargets();
    void setViewport();

    void Tick() override;
    void draw();

private:
    void setupCommandBuffer();
    void updateAfterSwapchainRecreate();

private:
    VkCommandPool m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;
    MainCameraRenderPass renderPass;
    std::vector<ImageAttachment> m_render_targets;

    void setupRenderpass();
};

#endif //XEXAMPLE_MAINCAMERARENDER_H
