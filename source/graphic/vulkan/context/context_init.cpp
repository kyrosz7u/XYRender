//
// Created by kyrosz7u on 3/29/23.
//
#include <iostream>
#include <set>
#include <algorithm>
#include "graphic/vulkan/vulkan_context.h"
#include "graphic/vulkan/vulkan_utils.h"

#define MACRO_XSTR(s) MACRO_STR(s)
#define MACRO_STR(s) #s


#if defined(_MSC_VER)
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#include <sdkddkver.h>
#include <Windows.h>
#elif defined(__GNUC__)
#include <stdlib.h>
#endif

using namespace VulkanAPI;

// debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void VulkanContext::initialize(GLFWwindow *window)
{
    _window = window;

#if defined(_MSC_VER)
    // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
    char const* vk_layer_path = MACRO_XSTR(VK_LAYER_PATH);
    SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
    SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
#elif defined(__GNUC__)
    #if defined(__MACH__)
    // https://stackoverflow.com/questions/32110858/how-to-set-environment-variable-in-mac-os-x-using-c-c
    char const* vk_layer_path    = MACRO_XSTR(VK_LAYER_PATH);
    // set MoltenVK icd in macos
    char const* vk_icd_filenames = MACRO_XSTR(VK_ICD_FILENAMES);
    setenv("VK_LAYER_PATH", vk_layer_path, 1);
    setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
    #endif
#else
#error Unknown Compiler
#endif

    createInstance();

    initializeDebugMessenger();

    createSurface();

    pickPhysicalDevice();

    createLogicalDevice();

    createCommandPool();

    createSwapchain();

    createSwapchainImageViews();
}

void VulkanContext::createInstance()
{
    if (!checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    // fill the application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanAPI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = m_vulkan_api_version;
    appInfo.pNext = NULL;


    // fill the instance create info and create
    VkInstanceCreateInfo instance_create_info = {};

    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &appInfo;
    instance_create_info.flags = 0;

    // get required extensions
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // add validation layer required extension
    std::vector<const char *> required_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

//  SDK 1.3.216 introduced a change in how devices are enumerated on MacOSX,
//  and your code must make use of the portability subset extension to discover your devices (again).
//  https://stackoverflow.com/questions/72789012/why-does-vkcreateinstance-return-vk-error-incompatible-driver-on-macos-despite
    required_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
//    required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    instance_create_info.ppEnabledExtensionNames = required_extensions.data();

    instance_create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
    instance_create_info.ppEnabledLayerNames = m_validation_layers.data();

    VkDebugUtilsMessengerCreateInfoEXT debugger_create_info = {};

    debugger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // 设置Debug的级别
    debugger_create_info.messageSeverity =
//            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // 设置Debug的消息类型
    debugger_create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugger_create_info.pfnUserCallback = debugCallback;

    instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugger_create_info;

    if ((vkCreateInstance(&instance_create_info, nullptr, &_instance)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanContext::initializeDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};

    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // 设置Debug的级别
    messengerCreateInfo.messageSeverity =
//            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // 设置Debug的消息类型
    messengerCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = debugCallback;
    messengerCreateInfo.pUserData = nullptr;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            _instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == NULL)
    {
        throw std::runtime_error("get vkCreateDebugUtilsMessengerEXT fault.");
    }

    if (func(_instance, &messengerCreateInfo, NULL, &m_debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("set debug messenger fault.");
    }

    _vkCmdBeginDebugUtilsLabelEXT =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
                    _instance, "vkCmdBeginDebugUtilsLabelEXT");
    _vkCmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
                    _instance, "vkCmdEndDebugUtilsLabelEXT");
}

void VulkanContext::createSurface()
{
    auto res = glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VulkanContext::pickPhysicalDevice()
{
    // 枚举物理设备
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, NULL);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find physical device .");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    std::vector<std::pair<int, VkPhysicalDevice>> ranked_physical_devices;
    // 4.2 choose a suitable physical device
    for (const auto &device: devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // 根据GPU的硬件类型来选择
        int score = 0;

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            score += 100;
        }
        ranked_physical_devices.push_back({score, device});
    }
    std::sort(ranked_physical_devices.begin(),
              ranked_physical_devices.end(),
              [](const std::pair<int, VkPhysicalDevice> &p1, const std::pair<int, VkPhysicalDevice> &p2)
              {
                  return p1 > p2;
              });

    for (const auto &device: ranked_physical_devices)
    {
        if (isDeviceSuitable(device.second))
        {
            _physical_device = device.second;
            break;
        }
    }

    if (_physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("no suitable device.");
    }
}

void VulkanContext::createLogicalDevice()
{
    _queue_indices = findQueueFamilies(_physical_device);
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos; // all queues that need to be created
    std::set<uint32_t> queue_families = {_queue_indices.graphicsFamily.value(), _queue_indices.presentFamily.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family: queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures physical_device_features = {};
    physical_device_features.samplerAnisotropy = VK_TRUE;
    physical_device_features.fragmentStoresAndAtomics = VK_TRUE;
    physical_device_features.independentBlend = VK_TRUE;
#if defined(__MACH__)
    // M1 Pro GPU不支持GeometryShader，原因暂时未知
    physical_device_features.geometryShader = VK_FALSE;
#else
    physical_device_features.geometryShader = VK_TRUE;
#endif


    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = m_device_extensions.data();
    device_create_info.enabledLayerCount = 0;

    if (vkCreateDevice(_physical_device, &device_create_info, nullptr, &_device) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create device");
    }
    vkGetDeviceQueue(_device, _queue_indices.graphicsFamily.value(), 0, &_graphics_queue);
    vkGetDeviceQueue(_device, _queue_indices.presentFamily.value(), 0, &_present_queue);

    // more efficient pointer
    _vkWaitForFences = (PFN_vkWaitForFences) vkGetDeviceProcAddr(_device, "vkWaitForFences");
    _vkResetFences = (PFN_vkResetFences) vkGetDeviceProcAddr(_device, "vkResetFences");
    _vkResetCommandPool = (PFN_vkResetCommandPool) vkGetDeviceProcAddr(_device, "vkResetCommandPool");
    _vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkGetDeviceProcAddr(_device, "vkBeginCommandBuffer");
    _vkEndCommandBuffer = (PFN_vkEndCommandBuffer) vkGetDeviceProcAddr(_device, "vkEndCommandBuffer");
    _vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) vkGetDeviceProcAddr(_device, "vkCmdBeginRenderPass");
    _vkCmdNextSubpass = (PFN_vkCmdNextSubpass) vkGetDeviceProcAddr(_device, "vkCmdNextSubpass");
    _vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) vkGetDeviceProcAddr(_device, "vkCmdEndRenderPass");
    _vkCmdBindPipeline = (PFN_vkCmdBindPipeline) vkGetDeviceProcAddr(_device, "vkCmdBindPipeline");
    _vkCmdSetViewport = (PFN_vkCmdSetViewport) vkGetDeviceProcAddr(_device, "vkCmdSetViewport");
    _vkCmdSetScissor = (PFN_vkCmdSetScissor) vkGetDeviceProcAddr(_device, "vkCmdSetScissor");
    _vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) vkGetDeviceProcAddr(_device, "vkCmdBindVertexBuffers");
    _vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) vkGetDeviceProcAddr(_device, "vkCmdBindIndexBuffer");
    _vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) vkGetDeviceProcAddr(_device, "vkCmdBindDescriptorSets");
    _vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) vkGetDeviceProcAddr(_device, "vkCmdDrawIndexed");
    _vkCmdClearAttachments = (PFN_vkCmdClearAttachments) vkGetDeviceProcAddr(_device, "vkCmdClearAttachments");

    _depth_image_format = findDepthFormat();
}

void VulkanContext::createCommandPool()
{
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = _queue_indices.graphicsFamily.value();

    // context专用cmdpool
    if (vkCreateCommandPool(_device, &command_pool_create_info, nullptr, &_command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create command pool");
    }
}

void VulkanContext::createSwapchain()
{
    // query all supports of this physical device
    SwapChainSupportDetails swapchain_support_details = querySwapChainSupport(_physical_device);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface, &swapchain_support_details.capabilities);
    // choose the best or fitting format
    VkSurfaceFormatKHR chosen_surface_format =
            chooseSwapchainSurfaceFormatFromDetails(swapchain_support_details.formats);
    // choose the best or fitting present mode
    VkPresentModeKHR chosen_presentMode = chooseSwapchainPresentModeFromDetails(swapchain_support_details.presentModes);
    // choose the best or fitting extent
    VkExtent2D chosen_extent = chooseSwapchainExtentFromDetails(swapchain_support_details.capabilities);

    uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
    if (swapchain_support_details.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support_details.capabilities.maxImageCount)
    {
        image_count = swapchain_support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;

    createInfo.minImageCount = image_count;
    createInfo.imageFormat = chosen_surface_format.format;
    createInfo.imageColorSpace = chosen_surface_format.colorSpace;
    createInfo.imageExtent = chosen_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {_queue_indices.graphicsFamily.value(), _queue_indices.presentFamily.value()};

    if (_queue_indices.graphicsFamily != _queue_indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapchain_support_details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = chosen_presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create swapchain khr");
    }
    
    vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, nullptr);
    _swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, _swapchain_images.data());

    _swapchain_image_format = chosen_surface_format.format;
    _swapchain_extent = chosen_extent;
}

void VulkanContext::createSwapchainImageViews()
{
    _swapchain_imageviews.resize(_swapchain_images.size());

    // create imageview (one for each this time) for all swapchain images
    for (size_t i = 0; i < _swapchain_images.size(); i++)
    {
        _swapchain_imageviews[i] = VulkanUtil::createImageView(_device,
                                                               _swapchain_images[i],
                                                               _swapchain_image_format,
                                                               VK_IMAGE_ASPECT_COLOR_BIT,
                                                               VK_IMAGE_VIEW_TYPE_2D,
                                                               1,
                                                               1);
    }
}

void VulkanContext::clearSwapchain()
{
    for (auto imageview: _swapchain_imageviews)
    {
        vkDestroyImageView(_device, imageview, NULL);
    }
    vkDestroySwapchainKHR(_device, _swapchain, NULL); // also swapchain images
}



