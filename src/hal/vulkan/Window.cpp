#include "Window.hpp"
#include "error_handling/Check.hpp"

namespace venture::vulkan {

Window::Window(int32_t width, int32_t height, std::string_view name, bool resizeable)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
    (void)resizeable;                           // TODO

    _window = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    checkf(_window != nullptr, "glfw create window");
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

vk::Result Window::create_surface_unique(vk::Instance instance, vk::UniqueSurfaceKHR *surface) noexcept
{
    VkSurfaceKHR c_surface;
    auto result = vk::Result(glfwCreateWindowSurface(instance, _window, nullptr, &c_surface));
    *surface = vk::UniqueSurfaceKHR(c_surface, instance);
    return result;
}

} // venture::vulkan

