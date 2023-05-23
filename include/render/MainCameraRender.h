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
    void initialize();

    void Tick() override;

private:
    void setupCommandBuffer();


private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> m_command_buffers;
    MainCameraRenderPass renderPass;
    std::vector<ImageAttachment> renderTargets;

    void setupRenderpass();
};

#endif //XEXAMPLE_MAINCAMERARENDER_H
