//
// Created by kyros on 2023/6/11.
//

#ifndef XEXAMPLE_SKYBOX_H
#define XEXAMPLE_SKYBOX_H

#include "subpass_base.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        struct SkyboxPassInitInfo : public SubPassInitInfo
        {

        };

        class SkyBoxPass : public SubPassBase
        {
        public:
            enum _skybox_pass_pipeline_layout_define
            {
                _skybox_pass_ubo_data_layout = 0,
                _skybox_pass_texture_layout,
                _skybox_pass_pipeline_layout_count
            };
            SkyBoxPass()
            {
                name = "skybox_subpass";
                m_descriptor_set_layouts.resize(_skybox_pass_pipeline_layout_count, VK_NULL_HANDLE);
            }

            void draw() override;

            void updateGlobalRenderDescriptorSet();

            void updateAfterSwapchainRecreate() override;
        };
    }
}

#endif //XEXAMPLE_SKYBOX_H
