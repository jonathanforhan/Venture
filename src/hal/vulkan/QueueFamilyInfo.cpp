#include "QueueFamilyInfo.hpp"
#include "error_handling/Check.hpp"

namespace venture::vulkan {

QueueFamilyInfo QueueFamilyInfo::get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    auto queue_family_tracker = QueueFamilyInfo();

    auto queue_family_props = physical_device.getQueueFamilyProperties();
    for (int32_t i = 0; const auto &queue_family_prop: queue_family_props)
    {
        if (queue_family_prop.queueCount <= 0)
            continue;

        // check graphics queue family support
        if (queue_family_prop.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            queue_family_tracker.graphics_family_index = i;
        }

        // check presentation queue family support
        vk::Bool32 presentation_support = false;
        check(physical_device.getSurfaceSupportKHR(i, surface, &presentation_support) == vk::Result::eSuccess);
        if (presentation_support)
        {
            queue_family_tracker.presentation_family_index = i;
        }

        if (queue_family_tracker.is_valid())
            break;

        i++;
    }

    return queue_family_tracker;
}

} // venture::vulkan
