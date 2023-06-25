//
// Created by kyros on 5/21/23.
//

#ifndef XEXAMPLE_MESH_FORWARD_LIGHT_H
#define XEXAMPLE_MESH_FORWARD_LIGHT_H

#include "subpass_base.h"
#include "render/resource/render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        struct MeshDeferLightingPassPassInitInfo : public SubPassInitInfo
        {
            std::vector<RenderMeshPtr> *render_mesh_list = nullptr;
        };

        class MeshDeferLightingPass : public SubPassBase
        {
        public:
            enum _mesh_defer_lighting_pipeline_layout_define
            {
                _mesh_defer_lighting_pass_ubo_data_layout = 0,
                _mesh_defer_lighting_pass_gbuffer_layout,
                _mesh_defer_lighting_pass_directional_light_shadow_layout,
                _mesh_defer_lighting_pass_pipeline_layout_count
            };
            MeshDeferLightingPass()
            {
                name = "mesh_defer_lighting_subpass";
                m_descriptor_set_layouts.resize(_mesh_defer_lighting_pass_pipeline_layout_count);
            }

            void draw() override;

            void updateGlobalRenderDescriptorSet();

            void updateAfterSwapchainRecreate() override;

        private:
            void initialize(SubPassInitInfo *subPassInitInfo) override;
            void setupPipeLineLayout();
            void setupDescriptorSet() override;
            void setupPipelines() override;
            VkDescriptorSet m_mesh_ubo_descriptor_set = VK_NULL_HANDLE;
        };
    }
}
#endif //XEXAMPLE_MESH_FORWARD_LIGHT_H
