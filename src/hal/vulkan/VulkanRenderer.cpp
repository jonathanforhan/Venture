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
        : _window(Window(800, 800, "Venture Engine")),
          _instance(),
          _surface(),
          _physical_device(),
          _logical_device(),
          _queue_family_info(),
          _graphics_queue(),
          _presentation_queue(),
          _swapchain()
{
    try {
        create_instance();
        create_surface();
        get_physical_device();
        create_logical_device();
        create_swapchain();
    } catch (const std::exception &e) {
        log(Error, e.what());
        throw e; // unwind stack and crash program
    }
}

void VulkanRenderer::render()
{
    // TODO
}

void VulkanRenderer::create_window(int32_t width, int32_t height, const char *name)
{
    // TODO
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
    }

    void *ici_next                    = _validation_layers_enabled ? &debug_create_info        : nullptr;
    uint32_t ici_enabled_layer_count  = _validation_layers_enabled ? _validation_layers.size() : 0;
    auto *ici_enabled_layer_names     = _validation_layers_enabled ? _validation_layers.data() : nullptr;

    vk::InstanceCreateInfo inst_create_info = {
            .sType = vk::StructureType::eInstanceCreateInfo,
            .pNext = ici_next,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = ici_enabled_layer_count,
            .ppEnabledLayerNames = ici_enabled_layer_names,
            .enabledExtensionCount = glfw_ext_count,
            .ppEnabledExtensionNames = glfw_exts,
    };

    _instance = vk::createInstanceUnique(inst_create_info);
}

void VulkanRenderer::create_surface()
{
    auto result = _window.create_surface_unique(*_instance, &_surface);
    checkf(result == vk::Result::eSuccess, "glfw create window surface");
}

void VulkanRenderer::create_logical_device()
{
    float priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> dev_queue_create_info_collection;
    std::set<int32_t> queue_family_indices = {
            _queue_family_info.graphics_family_index,
            _queue_family_info.presentation_family_index
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

    _logical_device = _physical_device.createDeviceUnique(dev_create_info);

    _logical_device->getQueue(_queue_family_info.graphics_family_index, 0, &_graphics_queue);
    _logical_device->getQueue(_queue_family_info.presentation_family_index, 0, &_presentation_queue);
}

void VulkanRenderer::create_swapchain()
{
    SwapchainInfo swapchain_info = SwapchainInfo::get_info(_physical_device, *_surface);
    vk::SurfaceFormatKHR surface_format = swapchain_info.optimal_surface_format();
    vk::PresentModeKHR present_mode = swapchain_info.optimal_present_mode();
    vk::Extent2D extent = swapchain_info.optimal_swap_extent(&_window);

    uint32_t sci_image_count = swapchain_info.surface_capabilities.minImageCount + 1;
    if (swapchain_info.surface_capabilities.maxImageCount != 0)
        sci_image_count = std::min(sci_image_count, swapchain_info.surface_capabilities.maxImageCount);

    uint32_t queue_family_array[] = {
            (uint32_t)_queue_family_info.graphics_family_index,
            (uint32_t)_queue_family_info.presentation_family_index
    };

    bool same_queue = _queue_family_info.graphics_family_index == _queue_family_info.presentation_family_index;

    auto sci_image_sharing_mode       = same_queue ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    auto sci_queue_family_index_count = same_queue ? uint32_t(0)                 : uint32_t(2);
    auto *sci_queue_family_indices    = same_queue ? nullptr                     : queue_family_array;

    vk::SwapchainCreateInfoKHR swapchain_create_info = {
            .sType = vk::StructureType::eSwapchainCreateInfoKHR,
            .surface = *_surface,
            .minImageCount = sci_image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = sci_image_sharing_mode,
            .queueFamilyIndexCount = sci_queue_family_index_count,
            .pQueueFamilyIndices = sci_queue_family_indices,
            .preTransform = swapchain_info.surface_capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = true,
            .oldSwapchain = nullptr
    };

    _swapchain = _logical_device->createSwapchainKHRUnique(swapchain_create_info);
}

void VulkanRenderer::get_physical_device()
{
    auto devs = _instance->enumeratePhysicalDevices();
    for (const auto &dev : devs)
    {
        auto queue_family_info = QueueFamilyInfo::get_info(dev, *_surface);
        if (queue_family_info.is_valid())
        {
            check_DEBUG(verify_physical_device_suitable(dev));
            _physical_device = dev;
            _queue_family_info = queue_family_info;
            return;
        }
    }
}

bool VulkanRenderer::verify_instance_extension_support(const char **exts, size_t exts_count)
{
    auto available_exts = vk::enumerateInstanceExtensionProperties();

    for (size_t i = 0; i < exts_count; i++)
    {
        auto match_ext = [=](vk::ExtensionProperties ext) -> bool {
            return std::strcmp(exts[i], ext.extensionName) == 0;
        };

        if (std::ranges::find_if(available_exts, match_ext) == available_exts.end())
        {
            logf(Error, "instance extension '%s' not supported", exts[i]);
            return false;
        }
    }

    return true;
}


bool VulkanRenderer::verify_device_extension_support(vk::PhysicalDevice physical_device)
{
    auto available_exts = physical_device.enumerateDeviceExtensionProperties();

    for (const auto &dev_ext : _device_extensions)
    {
        auto match_ext = [=](vk::ExtensionProperties ext) -> bool {
            return std::strcmp(dev_ext, ext.extensionName) == 0;
        };

        if (std::ranges::find_if(available_exts, match_ext) == available_exts.end())
        {
            logf(Error, "device extension '%s' not supported", dev_ext);
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::verify_instance_validation_layer_support()
{
    auto available_layers = vk::enumerateInstanceLayerProperties();

    for (const char *validation_layer : _validation_layers)
    {
        auto match_layer = [=](vk::LayerProperties ext) -> bool {
            return std::strcmp(validation_layer, ext.layerName) == 0;
        };

        if (std::ranges::find_if(available_layers, match_layer) == available_layers.end())
        {
            logf(Error, "validation layer '%s' not supported", validation_layer);
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::verify_physical_device_suitable(vk::PhysicalDevice physical_device)
{
    bool queue_family_valid = QueueFamilyInfo::get_info(physical_device, *_surface).is_valid();
    bool swap_chain_valid = SwapchainInfo::get_info(physical_device, *_surface).is_valid();
    bool dev_ext_support = verify_device_extension_support(physical_device);

    return queue_family_valid && swap_chain_valid && dev_ext_support;
}
