#pragma once

#include "VulkanApi.hpp"

namespace venture::vulkan {

class SwapchainImage
{
public:
    vk::Image image;
    vk::UniqueImageView image_view;
};

using SwapchainImageCollection = std::vector<SwapchainImage>;

} // venture::vulkan
