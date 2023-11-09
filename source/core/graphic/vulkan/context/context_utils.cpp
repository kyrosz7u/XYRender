//
// Created by kyrosz7u on 4/5/23.
//
#include "core/graphic/vulkan/vulkan_context.h"

#include <iostream>
#include <set>
#include <string>

using namespace VulkanAPI;

/* 注册一个回调函数，该函数在 Vulkan API 发生错误、产生警告或输出调试信息时被调用。
 * 该回调函数可以执行一些操作，如输出错误消息、记录调试信息或触发断点，以进行调试和错误排查。
 * */

VkResult VulkanContext::createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks*              pAllocator,
                                                             VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanContext::destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                          VkDebugUtilsMessengerEXT     debugMessenger,
                                                          const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice physical_device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(
            m_device_extensions.begin(), m_device_extensions.end());
    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

bool VulkanContext::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (auto validationLayerName : m_validation_layers)
    {
        bool support = false;
        for (const auto& supportedLayerName : availableLayers)
        {
            if (strcmp(supportedLayerName.layerName, validationLayerName) == 0)
            {
                support = true;
                break;
            }
        }
        if (support == false)
        {
            return false;
        }
    }
    return true;
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice physical_device)
{
    auto queue_indices           = findQueueFamilies(physical_device);
    bool is_extensions_supported = checkDeviceExtensionSupport(physical_device);
    bool is_swapchain_adequate   = false;
    if (is_extensions_supported)
    {
        SwapChainSupportDetails swapchain_support_details = querySwapChainSupport(physical_device);
        is_swapchain_adequate =
                !swapchain_support_details.formats.empty() && !swapchain_support_details.presentModes.empty();
    }

    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    if (!queue_indices.isComplete() || !is_swapchain_adequate || !physical_device_features.samplerAnisotropy)
    {
        return false;
    }

    return true;
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice physical_device) // for device and window
{
    QueueFamilyIndices indices;
    uint32_t           queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) // if support graphics command queue
        {
            indices.graphicsFamily = i;
        }

        VkBool32 is_present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
                                             i,
                                             _surface,
                                             &is_present_support); // if support window presentation
        if (is_present_support)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}

VkFormat VulkanContext::findDepthFormat()
{
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat VulkanContext::findDepthStencilFormat()
{
    return findSupportedFormat({VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat VulkanContext::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                                    VkImageTiling                tiling,
                                                    VkFormatFeatureFlags         features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("findSupportedFormat failed");
}

SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice physical_device)
{
    SwapChainSupportDetails details_result;

    // capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, _surface, &details_result.capabilities);

    // formats
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, _surface, &format_count, nullptr);
    if (format_count != 0)
    {
        details_result.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, _surface, &format_count, details_result.formats.data());
    }

    // present modes
    uint32_t presentmode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, _surface, &presentmode_count, nullptr);
    if (presentmode_count != 0)
    {
        details_result.presentModes.resize(presentmode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device, _surface, &presentmode_count, details_result.presentModes.data());
    }
    return details_result;
}

VkSurfaceFormatKHR VulkanContext::chooseSwapchainSurfaceFormatFromDetails(
        const std::vector<VkSurfaceFormatKHR>& available_surface_formats)
{
    for (const auto& surface_format : available_surface_formats)
    {
        // TODO: select the VK_FORMAT_B8G8R8A8_SRGB window format,
        // there is no need to do gamma correction in the fragment shader
        if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surface_format;
        }
    }
    return available_surface_formats[0];
}

VkPresentModeKHR VulkanContext::chooseSwapchainPresentModeFromDetails(
        const std::vector<VkPresentModeKHR>& available_present_modes)
{
    // VK_PRESENT_MODE_MAILBOX_KHR是三缓冲模式，m1pro gpu好像不支持
    // 故需要设置为VK_PRESENT_MODE_FIFO_KHR，设置为VK_PRESENT_MODE_IMMEDIATE_KHR
    // 会导致报错
    for (VkPresentModeKHR present_mode : available_present_modes)
    {
//        if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
//        {
//            return VK_PRESENT_MODE_MAILBOX_KHR;
//        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width =
                std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
                std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkCommandBuffer VulkanContext::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = _command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &command_buffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    _vkBeginCommandBuffer(command_buffer, &beginInfo);

    return command_buffer;
}

void VulkanContext::endSingleTimeCommands(VkCommandBuffer command_buffer)
{
    _vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &command_buffer;

    vkQueueSubmit(_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphics_queue);

    vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
}

void VulkanContext::clear()
{
    destroyDebugUtilsMessengerEXT(_instance, m_debug_messenger, nullptr);
}
