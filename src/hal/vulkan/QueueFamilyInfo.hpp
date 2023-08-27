#pragma once

#include "VulkanApi.hpp"

namespace venture::vulkan {

class VulkanRenderer;

/** Tracks queue family indices */
class QueueFamilyInfo
{
    int32_t graphics_family_index = -1;
    int32_t presentation_family_index = -1;

    [[nodiscard]] inline bool is_valid() const noexcept;
    static QueueFamilyInfo get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    friend VulkanRenderer;
};

bool QueueFamilyInfo::is_valid() const noexcept
{
    return (
            graphics_family_index >= 0 &&
            presentation_family_index >= 0
    );
}

} // venture::vulkan
