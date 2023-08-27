#pragma once

#include "VulkanApi.hpp"
#include <array>
#include <span>
#include "VulkanWindow.hpp"
#include "hal/IRenderer.hpp"
#include "QueueFamilyInfo.hpp"
#include "SwapchainInfo.hpp"
#include "SwapchainImage.hpp"

namespace venture::vulkan {

class VulkanRenderer : IRenderer
{
public:
    VulkanRenderer() = delete;
    /** VulkanRenderer does not own VulkanWindow the Engine does */
    explicit VulkanRenderer(VulkanWindow *window);

    void render() override;

private:
    void create_instance();
    void create_surface();
    void create_logical_device();
    void create_swapchain();
    vk::UniqueImageView create_image_view(vk::Image image, vk::Format format, vk::ImageAspectFlagBits flags) const;

    void get_physical_device();

    static bool verify_instance_extension_support(const std::span<const char *> extensions);
    static bool verify_device_extension_support(vk::PhysicalDevice physical_device);
    static bool verify_instance_validation_layer_support();
    bool verify_physical_device_suitable(vk::PhysicalDevice physical_device);

private:
    VulkanWindow *_window;
    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _logical_device;
    QueueFamilyInfo _queue_family_info;
    vk::Queue _graphics_queue;
    vk::Queue _presentation_queue;
    SwapchainInfo _swapchain_info;
    vk::UniqueSwapchainKHR _swapchain;
    SwapchainImageCollection _swapchain_images;

    constexpr static std::array<const char *, 1> _validation_layers = {
            "VK_LAYER_KHRONOS_validation",
    };

    constexpr static std::array<const char *, 1> _device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifndef V_DIST
    constexpr static bool _validation_layers_enabled = true;
#else
    constexpr static bool _validation_layers_enabled = false;
#endif
};

} // venture
