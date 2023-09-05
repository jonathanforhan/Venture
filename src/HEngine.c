#include "HEngine.h"
#include <stdbool.h>
#include <stdio.h>
#include "HLog.h"

#ifdef H_VULKAN
#include "hal/vulkan/VulkanWindow.h"
#include "hal/vulkan/VulkanRenderer.h"
typedef VulkanWindow window_t;
typedef VulkanRenderer renderer_t;
#define Window_create VulkanWindow_create
#define Renderer_create VulkanRenderer_create
#define Window_destroy VulkanWindow_destroy
#define Renderer_destroy VulkanRenderer_destroy
// speed up calls in main loop
#define Window_should_close(_Win) glfwWindowShouldClose(((VulkanWindow *)(_Win)->impl)->glfw_window)
#define Window_poll_events glfwPollEvents
#else
#error "Unsupported Renderer"
#endif

static bool initialized = false;
static window_t gWindow;
static renderer_t gRenderer;

HResult HEngine_create(HEngine *engine)
{
    HResult result;

    if (initialized)
    {
        H_LOG(HLogError, "Two instances of HEngine");
        return HResult_ERR;
    }
    initialized = true;

    engine->window.impl = &gWindow;
    engine->renderer.impl = &gRenderer;

    if ((result = Window_create(engine->window.impl, 800, 600, "Hydrogen Engine", HWindowOptsCentered)) != HResult_OK)
        goto abort_window;

    if ((result = Renderer_create(engine->renderer.impl, engine->window.impl)) != HResult_OK)
        goto abort_renderer;

    HWindow_show(&engine->window);

    return HResult_OK;

abort_renderer:
    Window_destroy(engine->window.impl);
abort_window:
    return result;
}

void HEngine_destroy(HEngine *engine)
{
    Renderer_destroy(engine->renderer.impl);
    Window_destroy(engine->window.impl);
}

HResult HEngine_run(HEngine *engine)
{
    while (!Window_should_close(&engine->window))
    {
        Window_poll_events();
    }

    return HResult_OK;
}
