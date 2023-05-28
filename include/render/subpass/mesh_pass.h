//
// Created by kyros on 5/21/23.
//

#ifndef XEXAMPLE_MESH_PASS_H
#define XEXAMPLE_MESH_PASS_H

#include "subpass_base.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        class MeshPass : public SubPassBase
        {
        public:
            MeshPass()
            {
                name = "mesh_subpass";
                // m_descriptor_list.resize(1);
            }

            void draw() override;

            void updateDescriptorSet();

            void updateAfterSwapchainRecreate() override;

        private:
            void initialize(SubPassInitInfo *subPassInitInfo) override;

            void setupDescriptorSetLayout() override;

            void setupDescriptorSet() override;

            void setupPipelines() override;

        };
    }
}
#endif //XEXAMPLE_MESH_PASS_H
