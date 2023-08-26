#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>

namespace venture::vulkan {

class SwapChainInfo
{
    SwapChainInfo() = default;
public:
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;

    [[nodiscard]] inline bool is_valid() const;
    static SwapChainInfo get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
};

bool SwapChainInfo::is_valid() const
{
    return !(surface_formats.empty() && present_modes.empty());
}

} // venture::vulkan
