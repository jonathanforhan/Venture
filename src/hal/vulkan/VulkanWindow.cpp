#include "VulkanWindow.hpp"
#include "error_handling/Check.hpp"

namespace venture::vulkan {

VulkanWindow::VulkanWindow(int32_t width, int32_t height, std::string_view name, bool resizeable)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
    (void)resizeable;                           // TODO

    _window = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    checkf(_window != nullptr, "glfw create window");
}

VulkanWindow::~VulkanWindow()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

vk::UniqueSurfaceKHR VulkanWindow::create_surface_unique(vk::Instance instance)
{
    VkSurfaceKHR surface;
    check(glfwCreateWindowSurface(instance, _window, nullptr, &surface) == VK_SUCCESS);
    return vk::UniqueSurfaceKHR(surface, instance);
}

} // venture::vulkan

