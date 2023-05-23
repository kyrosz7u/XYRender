//
// Created by kyros on 5/21/23.
//

#ifndef XEXAMPLE_MESH_H
#define XEXAMPLE_MESH_H

#include "graphic/vulkan/vulkan_subpass.h"

using namespace VulkanAPI;

namespace subPass
{
    class MeshPass: public VulkanSubPassBase
    {
    public:
        MeshPass()
        {
            name="mesh_subpass";
            m_descriptor_list.resize(1);

        }
        void initialize(SubPassInitInfo* subPassInitInfo) override;
        void setupDescriptorSetLayout() override;
        void updateDescriptorSet();
        void setupPipelines() override;
        void draw();
    };
}

#endif //XEXAMPLE_MESH_H
