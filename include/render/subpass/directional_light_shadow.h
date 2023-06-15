//
// Created by kyrosz7u on 2023/6/15.
//

#ifndef XEXAMPLE_DIRECTIONAL_LIGHT_SHADOW_H
#define XEXAMPLE_DIRECTIONAL_LIGHT_SHADOW_H

#include "subpass_base.h"

namespace RenderSystem
{
    namespace SubPass
    {
        struct DirectionalLightShadowPassInitInfo : public SubPassInitInfo
        {
        };

        class DirectionalLightShadowPass : public SubPassBase
        {
        public:
            enum _directional_light_shadow_pass_pipeline_layout_define
            {
                _mesh_ubo_data_layout = 0,
                _pipeline_layout_count
            };
            DirectionalLightShadowPass()
            {
                name= "directional_light_shadow_subpass";
            }

            void draw() override;

            void updateGlobalRenderDescriptorSet();

            void updateAfterSwapchainRecreate() override;

        private:
            void initialize(SubPassInitInfo *subPassInitInfo) override;
            void setupPipeLineLayout();
            void setupDescriptorSet() override;
            void setupPipelines() override;

            VkDescriptorSet m_dir_shadow_ubo_descriptor_set = VK_NULL_HANDLE;
        };
    }
}

#endif //XEXAMPLE_DIRECTIONAL_LIGHT_SHADOW_H
