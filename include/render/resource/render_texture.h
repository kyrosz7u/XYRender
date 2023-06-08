//
// Created by kyrosz7u on 2023/6/7.
//

#ifndef XEXAMPLE_RENDER_TEXTURE_H
#define XEXAMPLE_RENDER_TEXTURE_H

#include "core/graphic/vulkan/vulkan_context.h"
#include <vulkan/vulkan.h>
#include <string>
#include <memory>

namespace RenderSystem
{
    extern std::shared_ptr<VulkanAPI::VulkanContext> g_p_vulkan_context;

    class Texture2D;

    typedef std::shared_ptr<Texture2D> Texture2DPtr;

    class Texture2D
    {
    public:
        std::string name;
        std::string path;
        uint32_t    width, height;
        uint32_t    mip_levels;

        VkImage               image;
        VkDeviceMemory        memory;
        VkImageView           view;
        VkSampler             sampler;
        VkImageLayout         image_layout;
        VkDescriptorImageInfo descriptor;
    public:
        Texture2D(const std::string &path, const std::string &name, bool gen_mipmap = false);

        ~Texture2D();
    };
}

#endif //XEXAMPLE_RENDER_TEXTURE_H
