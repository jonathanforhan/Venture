#pragma once

#include "VulkanApi.hpp"

namespace venture::vulkan {

struct SwapchainImage
{
    vk::Image image;
    vk::UniqueImageView image_view;
};

} // venture::vulkan
