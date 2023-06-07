//
// Created by kyrosz7u on 2023/6/7.
//

#ifndef XEXAMPLE_RENDER_TEXTURE_H
#define XEXAMPLE_RENDER_TEXTURE_H

#include <vulkan/vulkan.h>
#include <stb_image.h>
#include <string>
#include "core/graphic/vulkan/vulkan_context.h"

namespace RenderSystem
{
    extern std::shared_ptr<VulkanAPI::VulkanContext> g_p_vulkan_context;
    class Texture2D;
    typedef std::shared_ptr<Texture2D> Texture2DPtr;
    class Texture2D
    {
    public:
        VkSampler      sampler;
        VkImage        image;
        VkImageLayout  image_layout;
        VkDeviceMemory memory;
        VkImageView    view;
        uint32_t       width, height;
        uint32_t       mip_levels;
    public:
        Texture2D(std::string texture_path, bool gen_mipmap = false);

        ~Texture2D();
    };
}

#endif //XEXAMPLE_RENDER_TEXTURE_H
