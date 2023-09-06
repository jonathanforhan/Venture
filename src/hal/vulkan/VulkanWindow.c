#include "VulkanWindow.h"
#include "HLog.h"
#include "HWindow.h"

HResult
VulkanWindow_create(VulkanWindow *vulkan_window, int32_t width, int32_t height, const char *title, uint32_t opts)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, (opts & HWindowOptsResizable) ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if ((vulkan_window->glfw_window = glfwCreateWindow(width, height, title, NULL, NULL)) == NULL)
    {
        glfwTerminate();
        H_LOG(HLogError, "Failed to create GLFW window");
        return HResult_ERR;
    }

    if (opts & HWindowOptsCentered)
    {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
        int x, y;
        glfwGetMonitorPos(monitor, &x, &y);

        // center the window
        x += (vidmode->width - width) / 2;
        y += (vidmode->height - height) / 2;

        glfwSetWindowPos(vulkan_window->glfw_window, x, y);
    }

    return HResult_OK;
}

void VulkanWindow_destroy(VulkanWindow *vulkan_window)
{
    glfwDestroyWindow(vulkan_window->glfw_window);
    glfwTerminate();
}

void VulkanWindow_show(VulkanWindow *vulkan_window)
{
    glfwShowWindow(vulkan_window->glfw_window);
}

bool VulkanWindow_should_close(VulkanWindow *vulkan_window)
{
    return glfwWindowShouldClose(vulkan_window->glfw_window);
}

void VulkanWindow_poll_events()
{
    glfwPollEvents();
}
