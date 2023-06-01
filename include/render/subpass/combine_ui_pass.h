//
// Created by kyrosz7u on 2023/6/1.
//

#ifndef EXAMPLEX_COMBINE_UI_PASS_H
#define EXAMPLEX_COMBINE_UI_PASS_H

#include "subpass_base.h"
#include "render/common_define.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        struct CombineUIPassInitInfo: public SubPassInitInfo
        {
            ImageAttachment* p_input_color_attachment;
            ImageAttachment* p_uipass_color_attachment;
        };

        class CombineUIPass: public SubPassBase
        {
        public:
            CombineUIPass()
            {
                name = "combine_ui_subpass";
            }

            void draw() override;
            void updateDescriptorSet();
            void updateAfterSwapchainRecreate() override;

        private: 
            void initialize(SubPassInitInfo *subpass_init_info) override;
            void setupDescriptorSetLayout() override;
            void setupDescriptorSet() override;
            void setupPipelines() override;

            ImageAttachment* m_p_input_color_attachment;
            ImageAttachment* m_p_uipass_color_attachment;
        };
    }
}
#endif //EXAMPLEX_COMBINE_UI_PASS_H
