//
// Created by kyrosz7u on 4/6/23.
//

#ifndef VULKANRENDER_VULKAN_UTILS_H
#define VULKANRENDER_VULKAN_UTILS_H

#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include "vulkan_context.h"

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

#include <iostream>
#include <unordered_map>
#include <vector>

namespace VulkanAPI
{
    class VulkanUtil
    {
    public:
        static uint32_t
        findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag);

        static std::string errorString(VkResult errorCode);

        static std::string physicalDeviceTypeString(VkPhysicalDeviceType type);

        static VkShaderModule createShaderModule(VkDevice device, const std::vector<unsigned char> &shader_code);

        static void createBuffer(std::shared_ptr<VulkanContext> p_context,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &buffer_memory);

        static void copyBuffer(std::shared_ptr<VulkanContext> p_context,
                               VkBuffer srcBuffer,
                               VkBuffer dstBuffer,
                               VkDeviceSize srcOffset,
                               VkDeviceSize dstOffset,
                               VkDeviceSize size);

        static void createImage(std::shared_ptr<VulkanContext> p_context,
                                uint32_t image_width,
                                uint32_t image_height,
                                VkFormat format,
                                VkImageTiling image_tiling,
                                VkImageUsageFlags image_usage_flags,
                                VkMemoryPropertyFlags memory_property_flags,
                                VkImage &image,
                                VkDeviceMemory &memory,
                                VkImageCreateFlags image_create_flags,
                                uint32_t array_layers,
                                uint32_t miplevels);

        static VkImageView createImageView(std::shared_ptr<VulkanContext> p_context,
                                           VkImage &image,
                                           VkFormat format,
                                           VkImageAspectFlags image_aspect_flags,
                                           VkImageViewType view_type,
                                           uint32_t layout_count,
                                           uint32_t miplevels);

        static VkImageView createImageView(VkDevice device,
                                           VkImage &image,
                                           VkFormat format,
                                           VkImageAspectFlags image_aspect_flags,
                                           VkImageViewType view_type,
                                           uint32_t layout_count,
                                           uint32_t miplevels);

        static void transitionImageLayout(std::shared_ptr<VulkanContext> p_context,
                                          VkImage image,
                                          VkImageLayout old_layout,
                                          VkImageLayout new_layout,
                                          uint32_t layer_count,
                                          uint32_t miplevels,
                                          VkImageAspectFlags aspect_mask_bits);

        static void copyBufferToImage(std::shared_ptr<VulkanContext> p_context,
                                      VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t layer_count);

        static void genMipmappedImage(std::shared_ptr<VulkanContext> p_context,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t mip_levels);

        static VkSampler
        getOrCreateMipmapSampler(VkPhysicalDevice physical_device, VkDevice device, uint32_t width, uint32_t height);

        static void destroyMipmappedSampler(VkDevice device);

        static VkSampler getOrCreateNearestSampler(VkPhysicalDevice physical_device, VkDevice device);

        static VkSampler getOrCreateLinearSampler(VkPhysicalDevice physical_device, VkDevice device);

        static void destroyNearestSampler(VkDevice device);

        static void destroyLinearSampler(VkDevice device);

    private:
        static std::unordered_map<uint32_t, VkSampler> m_mipmap_sampler_map;
        static VkSampler                               m_nearest_sampler;
        static VkSampler                               m_linear_sampler;
    };
} // namespace Pilot

#endif //VULKANRENDER_VULKAN_UTILS_H

