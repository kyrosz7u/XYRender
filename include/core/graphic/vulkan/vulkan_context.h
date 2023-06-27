//
// Created by kyrosz7u on 3/29/23.
//

#ifndef VULKANRENDER_VULKAN_CONTEXT_H
#define VULKANRENDER_VULKAN_CONTEXT_H

#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
#include <vector>
#include <optional>
#include <algorithm>
#include <functional>
#include <assert.h>

namespace VulkanAPI
{
    struct RenderCommandInfo
    {
        VkCommandBuffer  *p_current_command_buffer{nullptr};
        VkDescriptorPool *p_descriptor_pool{nullptr};
        const VkViewport *p_viewport{nullptr};
        const VkRect2D   *p_scissor{nullptr};
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    // api中所有与管线执行无关的接口封装
    class VulkanContext
    {
    public:
        GLFWwindow                 *_window = nullptr;
        VkInstance                 _instance;
        VkSurfaceKHR               _surface;
        VkPhysicalDevice           _physical_device;
        VkPhysicalDeviceProperties _physical_device_properties;

        QueueFamilyIndices _queue_indices;
        VkDevice           _device;
        VkQueue            _graphics_queue = VK_NULL_HANDLE;
        VkQueue            _present_queue  = VK_NULL_HANDLE;
        VkCommandPool      _command_pool   = VK_NULL_HANDLE;
        VkSwapchainKHR     _swapchain      = VK_NULL_HANDLE;

        void initialize(GLFWwindow *window);

        void clear();

        VkCommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(VkCommandBuffer command_buffer);

        // debug functions
        PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;
        // API function
        PFN_vkWaitForFences              _vkWaitForFences;
        PFN_vkResetFences                _vkResetFences;
        PFN_vkResetCommandPool           _vkResetCommandPool;
        PFN_vkBeginCommandBuffer         _vkBeginCommandBuffer;
        PFN_vkCmdExecuteCommands         _vkCmdExecuteCommands;
        PFN_vkEndCommandBuffer           _vkEndCommandBuffer;
        PFN_vkCmdBeginRenderPass         _vkCmdBeginRenderPass;
        PFN_vkCmdNextSubpass             _vkCmdNextSubpass;
        PFN_vkCmdEndRenderPass           _vkCmdEndRenderPass;
        PFN_vkCmdBindPipeline            _vkCmdBindPipeline;
        PFN_vkCmdSetViewport             _vkCmdSetViewport;
        PFN_vkCmdSetScissor              _vkCmdSetScissor;
        PFN_vkCmdBindVertexBuffers       _vkCmdBindVertexBuffers;
        PFN_vkCmdBindIndexBuffer         _vkCmdBindIndexBuffer;
        PFN_vkCmdBindDescriptorSets      _vkCmdBindDescriptorSets;
        PFN_vkCmdDraw                    _vkCmdDraw;
        PFN_vkCmdDrawIndexed             _vkCmdDrawIndexed;
        PFN_vkCmdClearAttachments        _vkCmdClearAttachments;
        PFN_vkAllocateDescriptorSets     _vkAllocateDescriptorSets;
        PFN_vkUpdateDescriptorSets       _vkUpdateDescriptorSets;
        PFN_vkFreeDescriptorSets         _vkFreeDescriptorSets;

        VkFormat                 _swapchain_image_format = VK_FORMAT_UNDEFINED;
        VkExtent2D               _swapchain_extent;
        std::vector<VkImage>     _swapchain_images;
        std::vector<VkImageView> _swapchain_imageviews;

        VkImage        _depth_image        = VK_NULL_HANDLE;
        VkDeviceMemory _depth_image_memory = VK_NULL_HANDLE;
        VkImageView    _depth_image_view   = VK_NULL_HANDLE;
        VkFormat       _depth_image_format = VK_FORMAT_UNDEFINED;

        static uint32_t const m_max_frames_in_flight = 3;
        uint32_t              m_current_frame_index  = 0;
        VkSemaphore           m_image_available_for_render_semaphores[m_max_frames_in_flight];
        VkSemaphore           m_image_finished_for_presentation_semaphores[m_max_frames_in_flight];
        VkFence               m_is_frame_in_flight_fences[m_max_frames_in_flight];

        void createSwapchain();

        void clearSwapchain();

        void recreateSwapChain();

        VkFormat findDepthFormat();

        VkFormat findDepthStencilFormat();

        void initSemaphoreObjects();

        void createSwapchainImageViews();

        void submitDrawSwapchainImageCmdBuffer(VkCommandBuffer *p_command_buffer);

        uint32_t getNextSwapchainImageIndex(std::function<void()> swapchainRecreateCallback);

        void presentSwapchainImage(uint32_t swapchain_image_index, std::function<void()> swapchainRecreateCallback);

        void waitForFrameInFlightFence();

    private:
        const std::vector<char const *> m_validation_layers  = {"VK_LAYER_KHRONOS_validation"};
        uint32_t                        m_vulkan_api_version = VK_API_VERSION_1_0;
        std::vector<char const *>       m_device_extensions  = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    private:
        void createInstance();

        void initializeDebugMessenger();

        void createSurface();

        void pickPhysicalDevice();

        void createLogicalDevice();

        void createCommandPool();

        void createAssetAllocator();

    private:
        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;

        bool checkValidationLayerSupport();

        std::vector<const char *> getRequiredExtensions();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkDebugUtilsMessengerEXT *pDebugMessenger);

        void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                           VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks *pAllocator);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device);

        bool checkDeviceExtensionSupport(VkPhysicalDevice physical_device);

        bool isDeviceSuitable(VkPhysicalDevice physical_device);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device);

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features);

        VkSurfaceFormatKHR
        chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR> &available_surface_formats);

        VkPresentModeKHR
        chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR> &available_present_modes);

        VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}
#endif //VULKANRENDER_VULKAN_CONTEXT_H