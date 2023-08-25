#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include "../IRenderer.hpp"
#include "QueueFamilyTracker.hpp"

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
    void create_logical_device();

    void get_physical_device();

    static bool verify_instance_extension_support(const char **exts, size_t exts_count);
    bool verify_instance_validation_layer_support();

    static void populate_debug_create_info(vk::DebugUtilsMessengerCreateInfoEXT *debug_create_info);

private:
    GLFWwindow *_window;
    vk::Instance _instance;
    vk::PhysicalDevice _physical_device;
    vk::Device _logical_device;
    vk::Queue _graphics_queue;

    std::array<const char *, 1> _validation_layers = {
            "VK_LAYER_KHRONOS_validation",
    };

#ifndef V_DIST
    constexpr static bool _validation_layers_enabled = true;
#else
    constexpr static bool _validation_layers_enabled = false;
#endif
};

//--- INLINE METHODS
bool VulkanRenderer::should_close() { return glfwWindowShouldClose(_window); }
void VulkanRenderer::poll_events() { glfwPollEvents(); }

} // venture
