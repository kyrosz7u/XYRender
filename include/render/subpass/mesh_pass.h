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

            std::vector<RenderMeshPtr> *m_p_visible_meshes;
        };
    }
}
#endif //XEXAMPLE_MESH_PASS_H
