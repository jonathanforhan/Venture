#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>

namespace venture::vulkan {

/** Tracks queue family indices */
class QueueFamilyTracker
{
    QueueFamilyTracker() {};
public:
    int32_t graphics_family_index = -1;

    inline bool is_valid() noexcept;
    static inline QueueFamilyTracker from_physical_device(vk::PhysicalDevice physical_device) noexcept;
};

bool QueueFamilyTracker::is_valid() noexcept
{
    if (graphics_family_index >= 0)
        return true;
    else
        return false;
}

QueueFamilyTracker QueueFamilyTracker::from_physical_device(vk::PhysicalDevice physical_device) noexcept
{
    auto queue_family_tracker = QueueFamilyTracker();

    auto queue_family_props = physical_device.getQueueFamilyProperties();
    for (int32_t i = 0; const auto &queue_family_prop : queue_family_props)
    {
        if (queue_family_prop.queueCount <= 0)
            continue;

        if (queue_family_prop.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            queue_family_tracker.graphics_family_index = i;
        }

        if (queue_family_tracker.is_valid())
            break;

        i++;
    }

    return queue_family_tracker;
}

} // venture::vulkan
