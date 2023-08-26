#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace venture::vulkan {

class SwapchainInfo;

class Window
{
public:
    explicit Window(int32_t width, int32_t height, std::string_view name = "Venture", bool resizeable = false);
    ~Window();

    [[nodiscard]] inline bool should_close() noexcept;
    vk::Result create_surface_unique(vk::Instance instance, vk::UniqueSurfaceKHR *surface) noexcept;

private:
    GLFWwindow *_window;

    friend SwapchainInfo;
};

bool Window::should_close() noexcept
{
    return glfwWindowShouldClose(_window);
}

} // venture::vulkan
