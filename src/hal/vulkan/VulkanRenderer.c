#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "VulkanRenderer.h"
#include "HLog.h"

#ifdef H_DIST
#define VALIDATION_LAYERS_ENABLED 0
#else
#define VALIDATION_LAYERS_ENABLED 1
#endif

//--- Variables

#define VERT_PATH "../spirv/vert.spv"
#define FRAG_PATH "../spirv/frag.spv"

static const char *VALIDATION_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation",
};

static const char *DEVICE_EXTENSIONS[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/* Pipeline Steps
 * 1 - create instance
 * 2 - create glfw window surface
 * 3 - find suitable physical device to support our extensions and surface ie the GPU
 * 4 - create logical device, this is where we find out graphics and present queue family indices
 * 5 - create swapchain and choose optimal format, mode, and extent
 */

//--- Forward Declaration

/* create vulkan instance, this is where instance validation layers are also setup if chosen
 * this functions will return error if instance extensions are not supported */
static VkResult create_instance(VulkanRenderer *vk_renderer);

/* create glfw window surface for respective platform */
static VkResult create_surface(VulkanRenderer *vk_renderer);

/* create logical device, this is also where we initialize graphics and present queue  */
static VkResult create_logical_device(VulkanRenderer *vk_renderer);

/* creates our swapchain, assigned our optimal values in swapchain_info using 'optimal_' functions*/
static VkResult create_swapchain(VulkanRenderer *vk_renderer);

/* get GPU, some verification of surface and extension support is done */
static VkResult get_physical_device(VulkanRenderer *vk_renderer);

/* this functions assumes a valid physical device is in the vk_renderer gotten through the
 * check_physical_device_queue_support functions */
static VkResult get_queue_family_indices(VulkanRenderer *vk_renderer);

/* mutates an array queue_indices_count to hold only unique values and returns the number of unique indices */
static size_t get_unique_device_queues(uint32_t queue_indices[], size_t queue_indices_count);

static VkResult check_instance_extension_support(const char **exts, uint32_t exts_count);
static VkResult check_physical_device_extension_support(VkPhysicalDevice physical_device);
static VkResult check_physical_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
static VkResult check_physical_device_queue_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
static VkResult check_physical_device_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

static VkSurfaceFormatKHR optimal_surface_format(VkSurfaceFormatKHR surface_formats[], uint32_t surface_format_count);
static VkPresentModeKHR  optimal_present_mode(VkPresentModeKHR *present_modes, uint32_t present_mode_count);
static VkExtent2D optimal_swapchain_extent(VkSurfaceCapabilitiesKHR surface_capabilities, GLFWwindow *window);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data);

//--- Helper
static uint32_t clamp(uint32_t d, uint32_t min, uint32_t max) {
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

//--- Methods

HResult VulkanRenderer_create(VulkanRenderer *vk_renderer, VulkanWindow *vk_window)
{
    VkResult result;

    vk_renderer->vulkan_window = vk_window;

    H_LOG(HLogVerbose, "Creating Vulkan Instance");
    if ((result = create_instance(vk_renderer)) != VK_SUCCESS)
        goto abort_instance;

    H_LOG(HLogVerbose, "Creating Window Surface");
    if ((result = create_surface(vk_renderer)) != VK_SUCCESS)
        goto abort_surface;

    H_LOG(HLogVerbose, "Getting Physical Device");
    if ((result = get_physical_device(vk_renderer)) != VK_SUCCESS)
        goto abort_physical_device;

    H_LOG(HLogVerbose, "Creating Logical Device");
    if ((result = create_logical_device(vk_renderer)) != VK_SUCCESS)
        goto abort_logical_device;

    H_LOG(HLogVerbose, "Creating Swap Chain");
    if ((result = create_swapchain(vk_renderer)) != VK_SUCCESS)
        goto abort_swapchain;

    return HResult_OK;

    // vkDestroySwapchainKHR(vk_renderer->logical_device, vk_renderer->swapchain, NULL);
abort_swapchain:
    vkDestroyDevice(vk_renderer->logical_device, NULL);
abort_logical_device:
abort_physical_device:
    vkDestroySurfaceKHR(vk_renderer->instance, vk_renderer->surface, NULL);
abort_surface:
    vkDestroyInstance(vk_renderer->instance, NULL);
abort_instance:
    H_LOG(HLogError, "Vulkan Creation Error %d", result);
    return HResult_ERR;
}

void VulkanRenderer_destroy(VulkanRenderer *vk_renderer)
{
    vkDestroySwapchainKHR(vk_renderer->logical_device, vk_renderer->swapchain, NULL);
    vkDestroyDevice(vk_renderer->logical_device, NULL);
    vkDestroySurfaceKHR(vk_renderer->instance, vk_renderer->surface, NULL);
    vkDestroyInstance(vk_renderer->instance, NULL);
}

//--- Static Function Impl

static VkResult create_instance(VulkanRenderer *vk_renderer)
{
    VkResult result;

    const VkApplicationInfo application_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Hydrogen Engine",
            .applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
            .pEngineName = "Hydrogen",
            .engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
    };

    const VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback
    };

    uint32_t glfw_ext_count;
    const char **glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    if ((result = check_instance_extension_support(glfw_exts, glfw_ext_count)) != VK_SUCCESS)
        return result;

#if VALIDATION_LAYERS_ENABLED
    VkInstanceCreateInfo instance_create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debug_create_info,
            .pApplicationInfo = &application_info,
            .enabledLayerCount = sizeof(VALIDATION_LAYERS) / sizeof(*VALIDATION_LAYERS),
            .ppEnabledLayerNames = VALIDATION_LAYERS,
            .enabledExtensionCount = glfw_ext_count,
            .ppEnabledExtensionNames = glfw_exts,
    };
#else
    VkInstanceCreateInfo instance_create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &application_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .enabledExtensionCount = glfw_ext_count,
            .ppEnabledExtensionNames = glfw_exts,
    };
#endif

    return vkCreateInstance(&instance_create_info, NULL, &vk_renderer->instance);
}

static VkResult create_surface(VulkanRenderer *vk_renderer)
{
    // Cross-platform extension for creating windows
    return glfwCreateWindowSurface(vk_renderer->instance, vk_renderer->vulkan_window->glfw_window, NULL,
                                   &vk_renderer->surface);
}

static VkResult create_logical_device(VulkanRenderer *vk_renderer)
{
    VkResult result;
    const float priority = 1.0f;

    // get the queue family indices on a now valid physical device
    if ((result = get_queue_family_indices(vk_renderer)) != VK_SUCCESS)
        return result;

    uint32_t queue_indices[] = {
            vk_renderer->graphics_family.index,
            vk_renderer->presentation_family.index
    };

    uint32_t unique_queue_indices = get_unique_device_queues(queue_indices, sizeof(queue_indices) / sizeof(*queue_indices));

    VkDeviceQueueCreateInfo *dev_queue_create_infos = malloc(unique_queue_indices * sizeof(*dev_queue_create_infos));
    if (!dev_queue_create_infos)
        return result;

    for (int32_t i = 0; i < unique_queue_indices; i++)
    {
        dev_queue_create_infos[i] = (VkDeviceQueueCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_indices[i],
                .queueCount = 1,
                .pQueuePriorities = &priority,
        };
    }

    const VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = unique_queue_indices,
            .pQueueCreateInfos = dev_queue_create_infos,
            .enabledExtensionCount = sizeof(DEVICE_EXTENSIONS) / sizeof(*DEVICE_EXTENSIONS),
            .ppEnabledExtensionNames = DEVICE_EXTENSIONS,
    };

    result = vkCreateDevice(vk_renderer->physical_device, &device_create_info, NULL, &vk_renderer->logical_device);

    free(dev_queue_create_infos);

    if (result != VK_SUCCESS)
        return result;

    vkGetDeviceQueue(vk_renderer->logical_device, vk_renderer->graphics_family.index, 0,
                     &vk_renderer->graphics_family.queue);

    vkGetDeviceQueue(vk_renderer->logical_device, vk_renderer->presentation_family.index, 0,
                     &vk_renderer->presentation_family.queue);

    return VK_SUCCESS;
}

static VkResult create_swapchain(VulkanRenderer *vk_renderer)
{
    VkPhysicalDevice physical_device = vk_renderer->physical_device;
    VkSurfaceKHR surface = vk_renderer->surface;

    //--- get extent
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    vk_renderer->swapchain_info.extent = optimal_swapchain_extent(surface_capabilities,
                                                                  vk_renderer->vulkan_window->glfw_window);
    //--- get surface format
    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);
    assert(surface_format_count > 0);

    VkSurfaceFormatKHR *surface_formats = malloc(surface_format_count * sizeof(VkSurfaceFormatKHR));
    if (!surface_formats)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

    vk_renderer->swapchain_info.surface_format = optimal_surface_format(surface_formats, surface_format_count);
    free(surface_formats);

    //--- present modes
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
    assert(present_mode_count > 0);

    VkPresentModeKHR *present_modes = malloc(present_mode_count * sizeof(VkSurfacePresentModeEXT));
    if (!present_modes)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);

    vk_renderer->swapchain_info.present_mode = optimal_present_mode(present_modes, present_mode_count);
    free(present_modes);

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.minImageCount > 0)
    {
        image_count = surface_capabilities.maxImageCount < image_count
                      ? surface_capabilities.minImageCount
                      : image_count;
    }

    uint32_t queue_family_indices[] = {
            vk_renderer->graphics_family.index,
            vk_renderer->presentation_family.index,
    };
    const bool same_queue = vk_renderer->graphics_family.index == vk_renderer->presentation_family.index;
    VkSharingMode sharing_mode = same_queue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT ;
    uint32_t indices_count     = same_queue ? 0                         : 2;
    uint32_t *indices          = same_queue ? NULL                      : queue_family_indices;

    VkSwapchainCreateInfoKHR swapchain_create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = vk_renderer->surface,
            .minImageCount = image_count,
            .imageFormat = vk_renderer->swapchain_info.surface_format.format,
            .imageColorSpace = vk_renderer->swapchain_info.surface_format.colorSpace,
            .imageExtent = vk_renderer->swapchain_info.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = sharing_mode,
            .queueFamilyIndexCount = indices_count,
            .pQueueFamilyIndices = indices,
            .preTransform = surface_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = vk_renderer->swapchain_info.present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
    };

    return vkCreateSwapchainKHR(vk_renderer->logical_device, &swapchain_create_info, NULL, &vk_renderer->swapchain);
}

static VkResult get_physical_device(VulkanRenderer *vk_renderer)
{
    uint32_t device_count;
    vkEnumeratePhysicalDevices(vk_renderer->instance, &device_count, NULL);

    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    if (!devices)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkEnumeratePhysicalDevices(vk_renderer->instance, &device_count, devices);

    for (int32_t i = 0; i < device_count; i++)
    {
        if (check_physical_device_suitable(devices[i], vk_renderer->surface) == VK_SUCCESS)
        {
            vk_renderer->physical_device = devices[i];
            break;
        }
    }

    free(devices);
    return VK_SUCCESS;
}

static VkResult get_queue_family_indices(VulkanRenderer *vk_renderer)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_renderer->physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_family_props = malloc(queue_family_count * sizeof(*queue_family_props));
    if (!queue_family_props)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkGetPhysicalDeviceQueueFamilyProperties(vk_renderer->physical_device, &queue_family_count, queue_family_props);

    // reset all indices
    vk_renderer->graphics_family.index = -1;
    vk_renderer->presentation_family.index = -1;

    for (int32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_family_props[i].queueCount <= 0)
            continue;

        // get graphics family
        if (queue_family_props->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            vk_renderer->graphics_family.index = i;
        }

        // get presentation family
        VkBool32 presentation_queue_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(
                vk_renderer->physical_device, i, vk_renderer->surface, &presentation_queue_support);
        if (presentation_queue_support)
        {
            vk_renderer->presentation_family.index = i;
        }

        if (vk_renderer->graphics_family.index >= 0 && vk_renderer->presentation_family.index >= 0)
            break;
    }
    // assert because we already checked with check device queue support
    assert(vk_renderer->graphics_family.index >= 0 && vk_renderer->presentation_family.index >= 0);

    free(queue_family_props);
    return VK_SUCCESS;
}

static size_t get_unique_device_queues(uint32_t queue_indices[], size_t queue_indices_count)
{
    uint32_t unique_elements = 0;

    for (int32_t i = 0; i < queue_indices_count; i++)
    {
        bool is_unique = true;

        int32_t j = 0;
        for (; j < unique_elements; j++)
        {
            if (queue_indices[i] == queue_indices[j])
            {
                is_unique = false;
                break;
            }
        }

        if (is_unique)
        {
            queue_indices[j] = queue_indices[i];
            unique_elements++;
        }
    }

    return unique_elements;
}

static VkResult check_instance_extension_support(const char **exts, uint32_t exts_count)
{
    uint32_t available_exts_count;
    vkEnumerateInstanceExtensionProperties(NULL, &available_exts_count, NULL);

    VkExtensionProperties *available_exts = malloc(available_exts_count * sizeof(*available_exts));
    if (!available_exts)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkEnumerateInstanceExtensionProperties(NULL, &available_exts_count, available_exts);

    bool has_ext;
    for (int32_t i = 0; i < exts_count; i++)
    {
        has_ext = false;
        for (int32_t j = 0; j < available_exts_count; j++)
        {
            if (strcmp(exts[i], available_exts[j].extensionName) == 0)
            {
                has_ext = true;
                break;
            }
        }

        if (!has_ext)
            break;
    }

    free(available_exts);
    return has_ext ? VK_SUCCESS : VK_ERROR_EXTENSION_NOT_PRESENT;
}

static VkResult check_physical_device_extension_support(VkPhysicalDevice physical_device)
{
    uint32_t available_exts_count;
    vkEnumerateDeviceExtensionProperties(physical_device, NULL, &available_exts_count, NULL);

    VkExtensionProperties *available_exts = malloc(available_exts_count * sizeof(*available_exts));
    if (!available_exts)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkEnumerateDeviceExtensionProperties(physical_device, NULL, &available_exts_count, available_exts);

    const uint32_t ext_count = sizeof(DEVICE_EXTENSIONS) / sizeof(*DEVICE_EXTENSIONS);
    bool has_ext;
    for (int32_t i = 0; i < ext_count; i++)
    {
        has_ext = false;
        for (int32_t j = 0; j < available_exts_count; j++)
        {
            if (strcmp(DEVICE_EXTENSIONS[i], available_exts[j].extensionName) == 0)
            {
                has_ext = true;
                break;
            }
        }

        if (!has_ext)
            break;
    }

    free(available_exts);
    return has_ext ? VK_SUCCESS : VK_ERROR_EXTENSION_NOT_PRESENT;
}

static VkResult check_physical_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    VkResult queue_support, device_exts_support, swapchain_support;

    if ((queue_support = check_physical_device_queue_support(physical_device, surface)) != VK_SUCCESS)
        return queue_support;

    if ((device_exts_support = check_physical_device_extension_support(physical_device)) != VK_SUCCESS)
        return device_exts_support;

    if ((swapchain_support = check_physical_device_swapchain_support(physical_device, surface)) != VK_SUCCESS)
        return swapchain_support;

    H_LOG(HLogVerbose, "Found suitable device: %s", device_properties.deviceName);
    return VK_SUCCESS;
}

static VkResult check_physical_device_queue_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_family_props = malloc(queue_family_count * sizeof(*queue_family_props));
    if (!queue_family_props)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_props);

    int32_t graphics_family_index = -1;
    int32_t presentation_family_index = -1;

    for (int32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_family_props[i].queueCount <= 0)
            continue;

        // check graphics queue
        if (queue_family_props->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_family_index = i;
        }

        // check presentation queue
        VkBool32 presentation_queue_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentation_queue_support);
        if (presentation_queue_support)
        {
            presentation_family_index = i;
        }

        if (graphics_family_index >= 0 && presentation_family_index >= 0)
        {
            free(queue_family_props);
            return VK_SUCCESS;
        }
    }

    free(queue_family_props);
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

static VkResult check_physical_device_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    /* Only check for number of capabilities / formats / modes we allocate no memory to store anything */
    VkResult result;

    //--- surface capabilities
    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
    if (result != VK_SUCCESS)
        return result;

    //--- surface formats
    uint32_t surface_format_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);
    if (result != VK_SUCCESS)
        return result;
    if (!surface_format_count)
        return VK_ERROR_FEATURE_NOT_PRESENT;

    //--- present modes
    uint32_t present_mode_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
    if (result != VK_SUCCESS)
        return result;
    if (!present_mode_count)
        return VK_ERROR_FEATURE_NOT_PRESENT;

    return VK_SUCCESS;
}

static VkSurfaceFormatKHR optimal_surface_format(VkSurfaceFormatKHR surface_formats[], uint32_t surface_format_count)
{
    /* prefer VK_FORMAT_R8G8B8A8_UNORM and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR */

    if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return (VkSurfaceFormatKHR) {
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR
        };
    }

    for (int32_t i = 0; i < surface_format_count; i++)
    {
        if (surface_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM &&
            surface_formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            return surface_formats[i];
    }

    return surface_formats[0];
}

static VkPresentModeKHR  optimal_present_mode(VkPresentModeKHR *present_modes, uint32_t present_mode_count)
{
    for (int32_t i = 0; i < present_mode_count; i++)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D optimal_swapchain_extent(VkSurfaceCapabilitiesKHR surface_capabilities, GLFWwindow *window)
{
    if (surface_capabilities.currentExtent.width != UINT32_MAX)
    {
        return surface_capabilities.currentExtent;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    // clamp width and height to max and min extents
    uint32_t width, height;
    width = clamp(w, surface_capabilities.minImageExtent.width, surface_capabilities.minImageExtent.width);
    height = clamp(h, surface_capabilities.minImageExtent.height, surface_capabilities.minImageExtent.height);

    return (VkExtent2D) {
            .width = width,
            .height = height,
    };
}

static VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data)
{
    (void)message_type;
    (void)user_data;
    switch (message_severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            H_LOG(HLogVerbose, "Vulkan API %s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            H_LOG(HLogInfo, "Vulkan API %s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            H_LOG(HLogWarning, "Vulkan API %s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            H_LOG(HLogError, "Vulkan API %s", callback_data->pMessage);
            break;
        default:
            break;
    }
    return false;
}
