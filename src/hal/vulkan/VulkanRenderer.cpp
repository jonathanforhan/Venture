#include "VulkanRenderer.hpp"
#include <iostream>
#include <ranges>
#include "error_handling/Check.hpp"

// -- C Callbacks
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data
)
{
    switch (message_severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            std::cout << "Debug Info -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            std::cout << "Debug Verbose -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cout << "Debug Warning -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cout << "Debug Error -- ";
            break;
    }
    std::cout << callback_data->pMessage << '\n';
    return false;
}

using namespace venture::vulkan;

VulkanRenderer::VulkanRenderer()
        : _window(nullptr),
          _instance(),
          _physical_device(),
          _logical_device(),
          _graphics_queue()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO

    try {
        create_window(800, 800, "Venture Engine");
        create_instance();
        get_physical_device();
        create_logical_device();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw e; // unwind stack and crash program
    }
}

VulkanRenderer::~VulkanRenderer()
{
    _logical_device.destroy();
    _instance.destroy();

    if (_window)
    {
        glfwDestroyWindow(_window);
    }
    glfwTerminate();
}

void VulkanRenderer::render()
{

}

void VulkanRenderer::create_window(int32_t width, int32_t height, const char *name)
{
    _window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    checkf(_window != nullptr, "create glfw window");
}

void VulkanRenderer::create_instance()
{
    if constexpr (_validation_layers_enabled)
        check(verify_instance_validation_layer_support());

    vk::ApplicationInfo app_info = {
            .sType = vk::StructureType::eApplicationInfo,
            .pApplicationName = "Venture Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Venture",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
    };

    uint32_t glfw_ext_count;
    const char **glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    check(verify_instance_extension_support(glfw_exts, glfw_ext_count));

    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {};
    void *ici_next = nullptr;
    uint32_t ici_enabled_layer_count = 0;
    const char *const *ici_enabled_layer_names = nullptr;
    if (_validation_layers_enabled)
    {
        populate_debug_create_info(&debug_create_info);
        ici_next = &debug_create_info;
        ici_enabled_layer_count = _validation_layers.size();
        ici_enabled_layer_names = _validation_layers.data();
    }

    vk::InstanceCreateInfo inst_create_info = {
            .sType = vk::StructureType::eInstanceCreateInfo,
            .pNext = ici_next,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = ici_enabled_layer_count,
            .ppEnabledLayerNames = ici_enabled_layer_names,
            .enabledExtensionCount = glfw_ext_count,
            .ppEnabledExtensionNames = glfw_exts,
    };

    vk::Result result = vk::createInstance(&inst_create_info, nullptr, &_instance);
    checkf(result == vk::Result::eSuccess, "create vulkan instance");
}

void VulkanRenderer::create_logical_device()
{
    auto queue_family_tracker = QueueFamilyTracker::from_physical_device(_physical_device);
    float priority = 1;

    vk::DeviceQueueCreateInfo dev_queue_create_info = {
            .sType = vk::StructureType::eDeviceQueueCreateInfo,
            .queueFamilyIndex = static_cast<uint32_t>(queue_family_tracker.graphics_family_index),
            .queueCount = 1,
            .pQueuePriorities = &priority,
    };

    vk::DeviceCreateInfo dev_create_info = {
            .sType = vk::StructureType::eDeviceCreateInfo,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &dev_queue_create_info,
    };

    auto result = _physical_device.createDevice(&dev_create_info, nullptr, &_logical_device);
    checkf(result == vk::Result::eSuccess, "create logical device");

    _logical_device.getQueue(queue_family_tracker.graphics_family_index, 0, &_graphics_queue);
}

void VulkanRenderer::get_physical_device()
{
    std::vector<vk::PhysicalDevice> devs = _instance.enumeratePhysicalDevices();
    for (const auto &dev : devs)
    {
        auto queue_family = QueueFamilyTracker::from_physical_device(dev);
        if (queue_family.is_valid())
        {
            _physical_device = dev;
            return;
        }
    }
}

bool VulkanRenderer::verify_instance_extension_support(const char **exts, size_t exts_count)
{
    uint32_t ext_props_count = 0;
    auto result = vk::enumerateInstanceExtensionProperties(nullptr, &ext_props_count, nullptr);
    checkf(result == vk::Result::eSuccess, "enumerate instance extensions properties");

    std::unique_ptr<vk::ExtensionProperties[]> ext_props(new vk::ExtensionProperties[ext_props_count]);
    result = vk::enumerateInstanceExtensionProperties(nullptr, &ext_props_count, ext_props.get());
    checkf(result == vk::Result::eSuccess, "enumerate instance extensions properties");

    for (uint32_t i : std::views::iota(size_t(0), exts_count))
    {
        bool has_ext = false;
        for (uint32_t j : std::views::iota(size_t(0), ext_props_count))
        {
            if (std::strcmp(exts[i], ext_props[j].extensionName) == 0)
            {
                has_ext = true;
                break;
            }
        }
        if (!has_ext)
            return false;
    }

    return true;
}

bool VulkanRenderer::verify_instance_validation_layer_support()
{
    uint32_t layer_count;
    auto result = vk::enumerateInstanceLayerProperties(&layer_count, nullptr);
    checkf(result == vk::Result::eSuccess, "enumerate instance layer properties");

    std::vector<vk::LayerProperties> layers(layer_count);
    result = vk::enumerateInstanceLayerProperties(&layer_count, layers.data());
    checkf(result == vk::Result::eSuccess, "enumerate instance layer properties");

    for (const char *validation_layer : _validation_layers)
    {
        bool found_layer = false;

        for (const auto &layer : layers)
        {
            if (std::strcmp(validation_layer, layer.layerName) == 0)
            {
                found_layer = true;
                break;
            }
        }

        if (!found_layer)
            return false;
    }

    return true;
}

void VulkanRenderer::populate_debug_create_info(vk::DebugUtilsMessengerCreateInfoEXT *debug_create_info)
{
    check(_validation_layers_enabled == true);
    debug_create_info->sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
    debug_create_info->messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debug_create_info->messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    debug_create_info->pfnUserCallback = debug_callback;
}
