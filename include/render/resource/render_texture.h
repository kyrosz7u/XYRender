//
// Created by kyrosz7u on 2023/6/7.
//

#ifndef XEXAMPLE_RENDER_TEXTURE_H
#define XEXAMPLE_RENDER_TEXTURE_H

#include <vulkan/vulkan.h>
#include <stb_image.h>

namespace RenderSystem
{
    class Texture2D
    {
    public:
        VkSampler sampler;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
    };
}

#endif //XEXAMPLE_RENDER_TEXTURE_H
