#ifndef HYDROGEN_SRC_HAL_VULKAN_VULKANRENDERER_H
#define HYDROGEN_SRC_HAL_VULKAN_VULKANRENDERER_H
#include "Api.h"
#include "VulkanWindow.h"
#include "HResult.h"

H_BEGIN

typedef struct VulkanRenderer
{
    /* window is owned by Engine NOT Renderer */
    VulkanWindow *vulkan_window;

    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    struct {
        int32_t index;
        VkQueue queue;
    } graphics_family;
    struct {
        int32_t index;
        VkQueue queue;
    } presentation_family;
    VkSurfaceKHR surface;
    struct {
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkExtent2D extent;
    } swapchain_info;
    VkSwapchainKHR swapchain;

} VulkanRenderer;

HResult VulkanRenderer_create(VulkanRenderer *vk_renderer, VulkanWindow *vk_window);
void VulkanRenderer_destroy(VulkanRenderer *vulkan_renderer);

H_END

#endif // HYDROGEN_SRC_HAL_VULKAN_VULKANRENDERER_H
