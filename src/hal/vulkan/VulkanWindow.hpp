#pragma once

#include "VulkanApi.hpp"
#include "hal/IWindow.hpp"

namespace venture::vulkan {

class SwapchainInfo;

class VulkanWindow : public IWindow
{
public:
    explicit VulkanWindow(int32_t width, int32_t height, std::string_view name = "Venture", bool resizeable = false);
    ~VulkanWindow();

    [[nodiscard]]
    inline bool should_close() noexcept override;
    inline void poll_events() noexcept override;
    [[nodiscard]]
    vk::UniqueSurfaceKHR create_surface_unique(vk::Instance instance) const;

private:
    GLFWwindow *_window;

    friend SwapchainInfo;
};

bool VulkanWindow::should_close() noexcept { return glfwWindowShouldClose(_window); }
void VulkanWindow::poll_events() noexcept { glfwPollEvents(); }

} // venture::vulkan
