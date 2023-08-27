#include "VulkanRenderer.hpp"
#include <iostream>
#include <ranges>
#include <set>
#include "error_handling/Check.hpp"
#include "error_handling/Log.hpp"
#include "Debug.hpp"

using namespace venture::vulkan;

VulkanRenderer::VulkanRenderer(VulkanWindow *window)
        : _window(window),
          _instance(),
          _surface(),
          _physical_device(),
          _logical_device(),
          _queue_family_info(),
          _graphics_queue(),
          _presentation_queue(),
          _swapchain_info(),
          _swapchain(),
          _swapchain_images()
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
    check_DEBUG(verify_instance_extension_support({ glfw_exts, glfw_ext_count }));

    static vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {
        .sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT,
        .messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debug_callback,
    };

    void *ici_next                   = _validation_layers_enabled ? &debug_create_info        : nullptr;
    uint32_t ici_enabled_layer_count = _validation_layers_enabled ? _validation_layers.size() : 0;
    auto *ici_enabled_layer_names    = _validation_layers_enabled ? _validation_layers.data() : nullptr;

    _instance = vk::createInstanceUnique({
            .sType = vk::StructureType::eInstanceCreateInfo,
            .pNext = ici_next,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = ici_enabled_layer_count,
            .ppEnabledLayerNames = ici_enabled_layer_names,
            .enabledExtensionCount = glfw_ext_count,
            .ppEnabledExtensionNames = glfw_exts,
    });
}

void VulkanRenderer::create_surface()
{
    _surface = _window->create_surface_unique(*_instance);
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

    _logical_device = _physical_device.createDeviceUnique({
            .sType = vk::StructureType::eDeviceCreateInfo,
            .queueCreateInfoCount = static_cast<uint32_t>(dev_queue_create_info_collection.size()),
            .pQueueCreateInfos = dev_queue_create_info_collection.data(),
            .enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size()),
            .ppEnabledExtensionNames = _device_extensions.data()
    });

    _logical_device->getQueue(_queue_family_info.graphics_family_index, 0, &_graphics_queue);
    _logical_device->getQueue(_queue_family_info.presentation_family_index, 0, &_presentation_queue);
}

void VulkanRenderer::create_swapchain()
{
    _swapchain_info = SwapchainInfo::get_info(_physical_device, *_surface, _window);

    uint32_t sci_image_count = _swapchain_info.surface_capabilities.minImageCount + 1;
    if (_swapchain_info.surface_capabilities.maxImageCount != 0)
        sci_image_count = std::min(sci_image_count, _swapchain_info.surface_capabilities.maxImageCount);

    uint32_t queue_family_array[] = {
            (uint32_t)_queue_family_info.graphics_family_index,
            (uint32_t)_queue_family_info.presentation_family_index
    };

    bool same_queue = _queue_family_info.graphics_family_index == _queue_family_info.presentation_family_index;

    auto sci_image_sharing_mode       = same_queue ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    auto sci_queue_family_index_count = same_queue ? uint32_t(0)                 : uint32_t(2);
    auto *sci_queue_family_indices    = same_queue ? nullptr                     : queue_family_array;

    _swapchain = _logical_device->createSwapchainKHRUnique({
            .sType = vk::StructureType::eSwapchainCreateInfoKHR,
            .surface = *_surface,
            .minImageCount = sci_image_count,
            .imageFormat = _swapchain_info.optimal->surface_format.format,
            .imageColorSpace = _swapchain_info.optimal->surface_format.colorSpace,
            .imageExtent = _swapchain_info.optimal->extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = sci_image_sharing_mode,
            .queueFamilyIndexCount = sci_queue_family_index_count,
            .pQueueFamilyIndices = sci_queue_family_indices,
            .preTransform = _swapchain_info.surface_capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = _swapchain_info.optimal->present_mode,
            .clipped = true,
            .oldSwapchain = nullptr
    });

    auto images = _logical_device->getSwapchainImagesKHR(*_swapchain);
    for (const auto image : images)
    {
        auto image_view = create_image_view(
                image, _swapchain_info.optimal->surface_format.format, vk::ImageAspectFlagBits::eColor);
        _swapchain_images.emplace_back(image, std::move(image_view));
    }
}

vk::UniqueImageView
VulkanRenderer::create_image_view(vk::Image image, vk::Format format, vk::ImageAspectFlagBits flags) const
{
    return _logical_device->createImageViewUnique({
            .sType = vk::StructureType::eImageViewCreateInfo,
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .components = {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity,
            },
            .subresourceRange = {
                    .aspectMask = flags,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
    });
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

bool VulkanRenderer::verify_instance_extension_support(const std::span<const char *> extensions)
{
    auto available_exts = vk::enumerateInstanceExtensionProperties();

    for (const auto ext : extensions)
    {
        auto match_ext = [=](auto prop) -> bool {
            return std::strcmp(ext, prop.extensionName) == 0;
        };

        if (std::ranges::find_if(available_exts, match_ext) == available_exts.end())
        {
            logf(Error, "instance extension '%s' not supported", ext);
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
    bool swap_chain_valid = SwapchainInfo::get_info(physical_device, *_surface, _window).is_valid();
    bool dev_ext_support = verify_device_extension_support(physical_device);

    return queue_family_valid && swap_chain_valid && dev_ext_support;
}
