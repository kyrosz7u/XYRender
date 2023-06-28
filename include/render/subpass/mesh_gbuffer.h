//
// Created by kyrosz7u on 2023/6/25.
//

#ifndef XEXAMPLE_MESH_GBUFFER_H
#define XEXAMPLE_MESH_GBUFFER_H

#include "subpass_base.h"
#include "render/resource/render_mesh.h"

using namespace VulkanAPI;

namespace RenderSystem
{
    namespace SubPass
    {
        struct MeshGBufferPassInitInfo : public SubPassInitInfo
        {
            std::vector<RenderMeshPtr> *render_mesh_list = nullptr;
        };

        class MeshGBufferPass : public SubPassBase
        {
        public:
            enum _mesh_pass_pipeline_layout_define
            {
                _mesh_pass_ubo_data_layout = 0,
                _mesh_pass_texture_layout,
                _mesh_pass_pipeline_layout_count
            };

            MeshGBufferPass()
            {
                name = "mesh_gbuffer_subpass";
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
            VkDescriptorSet m_mesh_ubo_descriptor_set = VK_NULL_HANDLE;

            void initialize(SubPassInitInfo *subPassInitInfo) override;

            void setupPipeLineLayout();

            void setupDescriptorSet() override;

            void setupPipelines() override;

            void drawSingleThread(VkCommandBuffer &command_buffer,
                                  VkCommandBufferInheritanceInfo &inheritance_info,
                                  uint32_t submesh_start_index,
                                  uint32_t submesh_end_index);
        };
    }
}

#endif //XEXAMPLE_MESH_GBUFFER_H
