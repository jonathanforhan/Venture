#include "VulkanRenderer.hpp"
#include <iostream>
#include <ranges>
#include <set>
#include "error_handling/Check.hpp"
#include "error_handling/Log.hpp"

// -- C Callbacks
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        [[maybe_unused]] void *user_data
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
            std::cerr << "Debug Error -- ";
            break;
        default:
            std::cout << "Debug Unknown -- ";
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
          _graphics_queue(),
          _presentation_queue(),
          _surface()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO

    try {
        create_window(800, 800, "Venture Engine");
        create_instance();
        create_surface();
        get_physical_device();
        create_logical_device();
    } catch (const std::exception &e) {
        log(Error, e.what());
        throw e; // unwind stack and crash program
    }
}

VulkanRenderer::~VulkanRenderer()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
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
    // TODO
}

void VulkanRenderer::create_window(int32_t width, int32_t height, const char *name)
{
    _window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    checkf(_window != nullptr, "glfw create window");
}

void VulkanRenderer::create_instance()
{
    if constexpr (_validation_layers_enabled)
    {
        check(verify_instance_validation_layer_support());
    }

    vk::ApplicationInfo app_info = {
            .sType = vk::StructureType::eApplicationInfo,
            .pApplicationName = "Venture Engine",
            .applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
            .pEngineName = "Venture",
            .engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
    };

    uint32_t glfw_ext_count;
    const char **glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    check(verify_instance_extension_support(glfw_exts, glfw_ext_count));

    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {};
    void *ici_next;
    uint32_t ici_enabled_layer_count;
    const char *const *ici_enabled_layer_names;

    if constexpr (_validation_layers_enabled)
    {
        debug_create_info.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        debug_create_info.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debug_create_info.messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        debug_create_info.pfnUserCallback = debug_callback;
        ici_next = &debug_create_info;
        ici_enabled_layer_count = _validation_layers.size();
        ici_enabled_layer_names = _validation_layers.data();
    }
    else
    {
        ici_next = nullptr;
        ici_enabled_layer_count = 0;
        ici_enabled_layer_names = nullptr;
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

void VulkanRenderer::create_surface()
{
    auto c_result = glfwCreateWindowSurface(_instance, _window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&_surface));
    checkf(c_result == VK_SUCCESS, "glfw create window surface");
}

void VulkanRenderer::create_logical_device()
{
    auto queue_family_info = QueueFamilyInfo::get_info(_physical_device, _surface);

    float priority = 1;

    std::vector<vk::DeviceQueueCreateInfo> dev_queue_create_info_collection;
    std::set<int32_t> queue_family_indices = {
            queue_family_info.graphics_family_index,
            queue_family_info.presentation_family_index
    };

    for (auto index : queue_family_indices)
    {
        dev_queue_create_info_collection.emplace_back(vk::DeviceQueueCreateInfo {
                .sType = vk::StructureType::eDeviceQueueCreateInfo,
                .queueFamilyIndex = static_cast<uint32_t>(index),
                .queueCount = 1,
                .pQueuePriorities = &priority,
        });
    }

    vk::DeviceCreateInfo dev_create_info = {
            .sType = vk::StructureType::eDeviceCreateInfo,
            .queueCreateInfoCount = static_cast<uint32_t>(dev_queue_create_info_collection.size()),
            .pQueueCreateInfos = dev_queue_create_info_collection.data(),
            .enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size()),
            .ppEnabledExtensionNames = _device_extensions.data()
    };

    auto result = _physical_device.createDevice(&dev_create_info, nullptr, &_logical_device);
    checkf(result == vk::Result::eSuccess, "create logical device");

    _logical_device.getQueue(queue_family_info.graphics_family_index, 0, &_graphics_queue);
    _logical_device.getQueue(queue_family_info.presentation_family_index, 0, &_presentation_queue);
}

void VulkanRenderer::get_physical_device()
{
    std::vector<vk::PhysicalDevice> devs = _instance.enumeratePhysicalDevices();
    for (const auto &dev : devs)
    {
        auto queue_family = QueueFamilyInfo::get_info(dev, _surface);
        if (queue_family.is_valid())
        {
            check_DEBUG(verify_physical_device_suitable(dev));
            _physical_device = dev;
            return;
        }
    }
}

bool VulkanRenderer::verify_instance_extension_support(const char **exts, size_t exts_count)
{
    auto ext_props = vk::enumerateInstanceExtensionProperties();

    for (uint32_t i : std::views::iota(size_t(0), exts_count))
    {
        auto match_ext = [=](vk::ExtensionProperties ext) -> bool {
            return std::strcmp(exts[i], ext.extensionName) == 0;
        };

        if (std::ranges::find_if(ext_props, match_ext) == ext_props.end())
        {
            logf(Error, "instance extension '%s' not supported", exts[i]);
            return false;
        }
    }

    return true;
}


bool VulkanRenderer::verify_device_extension_support(vk::PhysicalDevice physical_device)
{
    auto dev_ext_props = physical_device.enumerateDeviceExtensionProperties();

    for (const auto &dev_ext : _device_extensions)
    {
        auto match_ext = [=](vk::ExtensionProperties ext) -> bool {
            return std::strcmp(dev_ext, ext.extensionName) == 0;
        };

        if (std::ranges::find_if(dev_ext_props, match_ext) == dev_ext_props.end())
        {
            logf(Error, "device extension '%s' not supported", dev_ext);
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::verify_instance_validation_layer_support()
{
    auto layers = vk::enumerateInstanceLayerProperties();

    for (const char *validation_layer : _validation_layers)
    {
        auto match_layer = [=](vk::LayerProperties ext) -> bool {
            return std::strcmp(validation_layer, ext.layerName) == 0;
        };

        if (std::ranges::find_if(layers, match_layer) == layers.end())
        {
            logf(Error, "validation layer '%s' not supported", validation_layer);
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::verify_physical_device_suitable(vk::PhysicalDevice physical_device)
{
    bool queue_family_valid = QueueFamilyInfo::get_info(physical_device, _surface).is_valid();
    bool swap_chain_valid = SwapChainInfo::get_info(physical_device, _surface).is_valid();
    bool dev_ext_support = verify_device_extension_support(physical_device);

    return queue_family_valid && swap_chain_valid && dev_ext_support;
}
