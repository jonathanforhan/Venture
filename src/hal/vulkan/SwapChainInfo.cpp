#include "SwapChainInfo.hpp"
#include "error_handling/Check.hpp"

namespace venture::vulkan {

SwapChainInfo SwapChainInfo::get_info(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    SwapChainInfo swap_chain_info;

    auto result = physical_device.getSurfaceCapabilitiesKHR(surface, &swap_chain_info.surface_capabilities);
    checkf(result == vk::Result::eSuccess, "physical device get surface capabilities");
    swap_chain_info.surface_formats = physical_device.getSurfaceFormatsKHR(surface);
    swap_chain_info.present_modes = physical_device.getSurfacePresentModesKHR(surface);

    return swap_chain_info;
}

} // venture::vulkan