#pragma once

#include "VulkanApi.hpp"
#include "VulkanWindow.hpp"

namespace venture::vulkan {

class SwapchainInfo
{
public:
	SwapchainInfo()
		: surface_capabilities(),
		  surface_formats(),
	      present_modes(),
		  surface_format(),
          present_mode(vk::PresentModeKHR::eMailbox),
		  extent()
	{}

    vk::SurfaceCapabilitiesKHR surface_capabilities;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
    vk::SurfaceFormatKHR surface_format;
    vk::PresentModeKHR present_mode;
    vk::Extent2D extent;

    [[nodiscard]]
    inline bool is_valid() const;

    [[nodiscard]]
    static SwapchainInfo get_info(
            vk::PhysicalDevice physical_device,
            vk::SurfaceKHR surface,
            const VulkanWindow *window);

private:
    // Prefer
    //     Format       : VK_FORMAT_R8G8B8A8_UNORM
    //     ColorSpace   : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    [[nodiscard]]
    vk::SurfaceFormatKHR find_optimal_surface_format() const;

    // Prefer
    //     Present Mode : VK_PRESENT_MODE_MAILBOX_KHR
    [[nodiscard]]
    vk::PresentModeKHR find_optimal_present_mode() const;

    [[nodiscard]]
    vk::Extent2D find_optimal_extent(const VulkanWindow *window) const;
};

bool SwapchainInfo::is_valid() const
{
    return !(surface_formats.empty() && present_modes.empty());
}

} // venture::vulkan
