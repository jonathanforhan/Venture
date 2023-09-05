#include "HWindow.h"
#include <stdbool.h>
#include "hal/vulkan/VulkanWindow.h"

#ifdef H_VULKAN
#define Window_should_close VulkanWindow_should_close
#define Window_poll_events VulkanWindow_poll_events
#define Window_show VulkanWindow_show
#else
#error "Unsupported Renderer"
#endif

void HWindow_show(HWindow *window)
{
    Window_show(window->impl);
}

bool HWindow_should_close(HWindow *window)
{
    return Window_should_close(window->impl);
}

void HWindow_poll_events()
{
    Window_poll_events();
}
