#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>
#include <array>
#include "Window.hpp"
#include "../IRenderer.hpp"
#include "QueueFamilyInfo.hpp"
#include "SwapchainInfo.hpp"

namespace venture::vulkan {

class VulkanRenderer : IRenderer
{
public:
    explicit VulkanRenderer();

    inline bool should_close() override;
    inline void poll_events() override;
    void render() override;

private:
    void create_window(int32_t width, int32_t height, const char *name);
    void create_instance();
    void create_surface();
    void create_logical_device();
    void create_swapchain();

    void get_physical_device();

    static bool verify_instance_extension_support(const char **exts, size_t exts_count);
    static bool verify_device_extension_support(vk::PhysicalDevice physical_device);
    static bool verify_instance_validation_layer_support();
    bool verify_physical_device_suitable(vk::PhysicalDevice physical_device);

private:
    Window _window;
    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _logical_device;
    QueueFamilyInfo _queue_family_info;
    vk::Queue _graphics_queue;
    vk::Queue _presentation_queue;
    vk::UniqueSwapchainKHR _swapchain;

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

//--- Inline Methods
bool VulkanRenderer::should_close() { return _window.should_close(); }
void VulkanRenderer::poll_events() { glfwPollEvents(); }

} // venture
