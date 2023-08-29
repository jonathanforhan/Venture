#include "SwapchainInfo.hpp"
#include <limits>
#include "error_handling/Check.hpp"

namespace venture::vulkan {

SwapchainInfo SwapchainInfo::get_info(
        vk::PhysicalDevice physical_device,
        vk::SurfaceKHR surface,
        const VulkanWindow *window,
        bool find_optimal)
{
    SwapchainInfo swap_chain_info;

    auto result = physical_device.getSurfaceCapabilitiesKHR(surface, &swap_chain_info.surface_capabilities);
    checkf(result == vk::Result::eSuccess, "physical device get surface capabilities");
    swap_chain_info.surface_formats = physical_device.getSurfaceFormatsKHR(surface);
    swap_chain_info.present_modes = physical_device.getSurfacePresentModesKHR(surface);

    if (find_optimal)
    {
        swap_chain_info.optimal = {
            .surface_format = swap_chain_info.find_optimal_surface_format(),
            .present_mode = swap_chain_info.find_optimal_present_mode(),
            .extent = swap_chain_info.find_optimal_extent(window),
        };
    }

    return swap_chain_info;
}

vk::SurfaceFormatKHR SwapchainInfo::find_optimal_surface_format() const
{
    // if all formats present
    if (surface_formats.size() == 1 && surface_formats[0].format == vk::Format::eUndefined)
        return { vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    auto find_preferred = [](vk::SurfaceFormatKHR fmt) -> bool {
        return fmt.format == vk::Format::eR8G8B8A8Unorm && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    };

    return std::ranges::find_if(surface_formats, find_preferred) != surface_formats.end()
        ? vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear)
        : surface_formats.at(0); // any random format
}

vk::PresentModeKHR SwapchainInfo::find_optimal_present_mode() const
{
    return std::ranges::find(present_modes, vk::PresentModeKHR::eMailbox) != present_modes.end()
        ? vk::PresentModeKHR::eMailbox
        : vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapchainInfo::find_optimal_extent(const VulkanWindow *window) const
{
    // if surface width == INT_MAX then window extent can vary else it is window dimensions
    if (surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
        return surface_capabilities.currentExtent;

    uint32_t width, height;
    glfwGetFramebufferSize(window->_window, (int *)&width, (int *)&height);

    width  = std::clamp(width,  surface_capabilities.minImageExtent.width,  surface_capabilities.maxImageExtent.width);
    height = std::clamp(height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

    return vk::Extent2D(width, height);
}

} // venture::vulkan