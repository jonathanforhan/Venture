#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>

namespace venture::vulkan {

/** Tracks queue family indices */
class QueueFamilyInfo
{
    QueueFamilyInfo() = default;
public:
    int32_t graphics_family_index = -1;
    int32_t presentation_family_index = -1;

    [[nodiscard]] inline bool is_valid() const noexcept;
    static QueueFamilyInfo get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
};

bool QueueFamilyInfo::is_valid() const noexcept
{
    return (
            graphics_family_index >= 0 &&
            presentation_family_index >= 0
    );
}

} // venture::vulkan
