//
// Created by kyrosz7u on 4/7/23.
//

#ifndef VULKANRENDER_VULKAN_MANAGER_H
#define VULKANRENDER_VULKAN_MANAGER_H

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vulkan/vulkan.h"
#include "vulkan_context.h"

#include <iostream>

namespace VulkanAPI
{

// Vk中图形或计算管线执行接口的封装
    class VulkanManager
    {
    public:
        VulkanManager();

        // clear module resource
        void clear();

        // initialize vulkan from io->window
        int initialize(std::shared_ptr<VulkanContext> _context);


        // rendering config
        static bool m_enable_validation_Layers;
        static bool m_enable_debug_utils_label;
        static bool m_enable_point_light_shadow;

    private:
        // initialize render passes
        bool initializeRenderPass();
        // initialize command pool
        bool initializeCommandPool();
        // description pool for uniform buffer and image sampler
        bool initializeDescriptorPool();

        // allocate command buffer: for drawing commands
        bool initializeCommandBuffers();
        // swapchain clear or recreate
        void clearSwapChain();
        // recreate swapchain
        bool recreateSwapChain();

        // prepare context
        void prepareContext();

        int getCurrentSwapchainImageIndex();

        // vulkan context include device creation, default command buffer, etc
        std::shared_ptr<VulkanContext> m_vulkan_context;

        static uint32_t const m_max_frames_in_flight = 3;
        uint32_t              m_current_frame_index  = 0;

        // global descriptor pool
        VkDescriptorPool m_descriptor_pool;

        // viewport info
        VkViewport m_viewport = {0, 0, 1280, 720, 0, 1};
        VkRect2D   m_scissor  = {{0, 0}, {1280, 720}};

        VkCommandPool   m_command_pools[m_max_frames_in_flight];
        VkCommandBuffer m_command_buffers[m_max_frames_in_flight];
        VkSemaphore     m_image_available_for_render_semaphores[m_max_frames_in_flight];
        VkSemaphore     m_image_finished_for_presentation_semaphores[m_max_frames_in_flight];
        VkFence         m_is_frame_in_flight_fences[m_max_frames_in_flight];

        uint32_t m_max_vertex_blending_mesh_count{ 256 };
        uint32_t m_max_material_count{ 256 };

        // load IBL

        // load color grading
//        void updateGlobalTexturesForColorGrading(PColorGradingResourceData& color_grading_resource_data);



        // cached mesh
//        std::map<size_t, VulkanMesh> m_vulkan_meshes;
        // cached material
//        std::map<size_t, VulkanPBRMaterial> m_vulkan_pbr_materials;

        // data uploading functions
//        VulkanMesh&
//        syncMesh(struct RenderMesh const& mesh, bool has_skeleton_binding_handle, class PilotRenderer* pilot_renderer);
//        void               updateMeshData(bool                                          has_skeleton_binding_handle,
//                                          uint32_t                                      index_buffer_size,
//                                          void*                                         index_buffer_data,
//                                          uint32_t                                      vertex_buffer_size,
//                                          struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
//                                          uint32_t                                      joint_binding_buffer_size,
//                                          struct Mesh_VertexBinding const*              joint_binding_buffer_data,
//                                          VulkanMesh&                                   now_mesh);
//        bool               updateVertexBuffer(bool                                          has_skeleton_binding_handle,
//                                              uint32_t                                      vertex_buffer_size,
//                                              struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
//                                              uint32_t                                      joint_binding_buffer_size,
//                                              struct Mesh_VertexBinding const*              joint_binding_buffer_data,
//                                              uint32_t                                      index_buffer_size,
//                                              uint16_t*                                     index_buffer_data,
//                                              VulkanMesh&                                   now_mesh);
//        bool               updateIndexBuffer(uint32_t index_buffer_size, void* index_buffer_data, VulkanMesh& now_mesh);
//        VulkanPBRMaterial& syncMaterial(struct Material const& material, class PilotRenderer* pilot_renderer);
//        void               updateTextureImageData(const PTextureDataToUpdate& texture_data);
//        bool               initializeTextureImage(VkImage&           image,
//                                                  VkImageView&       image_view,
//                                                  VmaAllocation&     image_allocation,
//                                                  uint32_t           texture_image_width,
//                                                  uint32_t           texture_image_height,
//                                                  void*              texture_image_pixels,
//                                                  PILOT_PIXEL_FORMAT texture_image_format,
//                                                  uint32_t           miplevels = 0);
    };
};

#endif //VULKANRENDER_VULKAN_MANAGER_H
