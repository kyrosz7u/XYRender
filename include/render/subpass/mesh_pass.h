//
// Created by kyros on 5/21/23.
//

#ifndef XEXAMPLE_MESH_PASS_H
#define XEXAMPLE_MESH_PASS_H

#include "subpass_base.h"
#include "render/resource/render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        enum _mesh_pass_descriptor_set_define
        {
            _mesh_pass_ubo_data_descriptor_set = 0,
            _mesh_pass_texture_sampler_descriptor_set,
            _mesh_pass_descriptor_set_count
        };
        struct MeshPassInitInfo : public SubPassInitInfo
        {
            std::vector<RenderMeshPtr> *render_mesh_list = nullptr;
        };

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
