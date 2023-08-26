#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "Window.hpp"

namespace venture::vulkan {

class SwapchainInfo
{
    SwapchainInfo() = default;
public:
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;

    [[nodiscard]] inline bool is_valid() const;
    static SwapchainInfo get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    // Prefer
    //     Format       : VK_FORMAT_R8G8B8A8_UNORM
    //     ColorSpace   : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    [[nodiscard]] vk::SurfaceFormatKHR optimal_surface_format() const;

    // Prefer
    //     Present Mode : VK_PRESENT_MODE_MAILBOX_KHR
    [[nodiscard]] vk::PresentModeKHR optimal_present_mode() const;

    vk::Extent2D optimal_swap_extent(Window *window) const;
};

bool SwapchainInfo::is_valid() const
{
    return !(surface_formats.empty() && present_modes.empty());
}

} // venture::vulkan
