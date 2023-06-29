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
        struct MeshForwardLightingPassPassInitInfo : public SubPassInitInfo
        {
            std::vector<RenderMeshPtr> *render_mesh_list = nullptr;
        };

        class MeshForwardLightingPass : public SubPassBase
        {
        public:
            enum _mesh_pass_pipeline_layout_define
            {
                _mesh_pass_ubo_data_layout = 0,
                _mesh_pass_texture_layout,
                _mesh_pass_directional_light_shadow_layout,
                _mesh_pass_pipeline_layout_count
            };
            MeshForwardLightingPass()
            {
                name = "mesh_forward_lighting_subpass";
                m_descriptor_set_layouts.resize(_mesh_pass_pipeline_layout_count);
            }

            void draw() override;

            void drawMultiThreading(ThreadPool &thread_pool,
                                    std::vector<RenderThreadData> &thread_data,
                                    VkCommandBufferInheritanceInfo &inheritance_info,
                                    uint32_t command_buffer_index,
                                    uint32_t thread_start_index,
                                    uint32_t thread_count) override;

            void updateGlobalRenderDescriptorSet();

            void updateAfterSwapchainRecreate() override;

        private:
            void initialize(SubPassInitInfo *subPassInitInfo) override;
            void setupPipeLineLayout();
            void setupDescriptorSet() override;
            void setupPipelines() override;
            VkDescriptorSet m_mesh_ubo_descriptor_set = VK_NULL_HANDLE;

            void drawSingleThread(VkCommandBuffer &command_buffer, VkCommandBufferInheritanceInfo &inheritance_info,
                                  uint32_t submesh_start_index, uint32_t submesh_end_index);
        };
    }
}
#endif //XEXAMPLE_MESH_FORWARD_LIGHT_H
