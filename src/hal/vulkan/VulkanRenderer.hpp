#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include "../IRenderer.hpp"
#include "QueueFamilyInfo.hpp"
#include "SwapChainInfo.hpp"

namespace venture::vulkan {

class VulkanRenderer : IRenderer
{
public:
    explicit VulkanRenderer();
    ~VulkanRenderer();

    inline bool should_close() override;
    inline void poll_events() override;
    void render() override;

private:
    void create_window(int32_t width, int32_t height, const char *name);
    void create_instance();
    void create_surface();
    void create_logical_device();

    void get_physical_device();

    static bool verify_instance_extension_support(const char **exts, size_t exts_count);
    static bool verify_device_extension_support(vk::PhysicalDevice physical_device);
    static bool verify_instance_validation_layer_support();
    bool verify_physical_device_suitable(vk::PhysicalDevice physical_device);

private:
    GLFWwindow *_window;
    vk::Instance _instance;
    vk::PhysicalDevice _physical_device;
    vk::Device _logical_device;
    vk::Queue _graphics_queue;
    vk::Queue _presentation_queue;
    vk::SurfaceKHR _surface;

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
bool VulkanRenderer::should_close() { return glfwWindowShouldClose(_window); }
void VulkanRenderer::poll_events() { glfwPollEvents(); }

} // venture
