  //
  // Created by kyrosz7u on 2023/6/1.
  //

#ifndef EXAMPLEX_UI_PASS_H
#define EXAMPLEX_UI_PASS_H

#include "subpass_base.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        struct UIPassInitInfo: public SubPassInitInfo
        {
            
        };

        class UIPass: public SubPassBase
        {
        public: 
            UIPass()
            {
                name = "ui_subpass";
            }

            void draw() override;
            void updateDescriptorSet();
            void updateAfterSwapchainRecreate() override;

        private: 
            void initialize(SubPassInitInfo *subpass_init_info) override;
            void UIPass::initializeUIRenderBackend();
        };
    }
}

#endif  //EXAMPLEX_UI_PASS_H
