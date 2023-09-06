#ifndef HYDROGEN_SRC_HAL_VULKAN_VULKANWINDOW_H
#define HYDROGEN_SRC_HAL_VULKAN_VULKANWINDOW_H
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "HApi.h"
#include "HResult.h"

H_BEGIN

typedef struct VulkanWindow
{
    GLFWwindow *glfw_window;
} VulkanWindow;

HResult VulkanWindow_create(VulkanWindow *vulkan_window, int32_t width, int32_t height, const char *title, uint32_t opts);
void VulkanWindow_destroy(VulkanWindow *vulkan_window);

void VulkanWindow_show(VulkanWindow *vulkan_window);
bool VulkanWindow_should_close(VulkanWindow *vulkan_window);
void VulkanWindow_poll_events();

H_END

#endif // HYDROGEN_SRC_HAL_VULKAN_VULKANWINDOW_H
