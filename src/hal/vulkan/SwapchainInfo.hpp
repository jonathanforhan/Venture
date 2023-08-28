#pragma once

#include "VulkanApi.hpp"
#include "VulkanWindow.hpp"
#include <optional>

namespace venture::vulkan {

class SwapchainInfo
{
public:
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
    typedef struct {
        vk::SurfaceFormatKHR surface_format;
        vk::PresentModeKHR present_mode;
        vk::Extent2D extent;
    } optimal_t;
    // only use optimal values if find_optimal was set to true in get_info
    std::optional<optimal_t> optimal = std::nullopt;

    [[nodiscard]]
    inline bool is_valid() const;

    [[nodiscard]]
    static SwapchainInfo get_info(
            vk::PhysicalDevice physical_device,
            vk::SurfaceKHR surface,
            const VulkanWindow *window,
            bool find_optimal = true);

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
