#pragma once

#include "VulkanApi.hpp"
#include <array>
#include <span>
#include "VulkanWindow.hpp"
#include "hal/IRenderer.hpp"
#include "QueueFamilyInfo.hpp"
#include "SwapchainInfo.hpp"
#include "SwapchainImage.hpp"

namespace venture::vulkan {

class VulkanRenderer : IRenderer
{
public:
    VulkanRenderer() = delete;
    /** VulkanRenderer does not own VulkanWindow the Engine does */
    explicit VulkanRenderer(VulkanWindow *window);

    void render() override;

private:
    // mutate internal state of renderer
    void create_instance();
    void create_surface();
    void create_logical_device();
    void create_swapchain();
    void create_render_pass();
    void create_graphics_pipeline();

    // make objects without mutating renderer
    [[nodiscard]]
    vk::UniqueImageView make_image_view(vk::Image image, vk::Format format, vk::ImageAspectFlagBits flags) const;
    [[nodiscard]]
    vk::UniqueShaderModule make_shader_module(const char *path) const;

    // assign existing data to internal state, nothing created
    void retrieve_physical_device();

    // verify, non-mutating
    static bool verify_instance_extension_support(std::span<const char *> extensions);
    static bool verify_device_extension_support(vk::PhysicalDevice physical_device);
    static bool verify_instance_validation_layer_support();
    [[nodiscard]]
    bool verify_physical_device_suitable(vk::PhysicalDevice physical_device) const;

private:
    VulkanWindow *_window;
    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _logical_device;
    QueueFamilyInfo _queue_family_info;
    vk::Queue _graphics_queue;
    vk::Queue _presentation_queue;
    SwapchainInfo _swapchain_info;
    vk::UniqueSwapchainKHR _swapchain;
    std::vector<SwapchainImage> _swapchain_images;
    vk::UniqueRenderPass _render_pass;
    vk::UniquePipelineLayout _pipeline_layout;
    vk::UniquePipeline _graphics_pipeline;

    constexpr static const char *VERT_PATH = "../spirv/vert.spv";
    constexpr static const char *FRAG_PATH = "../spirv/frag.spv";
    constexpr static std::array<const char *, 1> VALIDATION_LAYERS = {
            "VK_LAYER_KHRONOS_validation",
    };
    constexpr static std::array<const char *, 1> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifndef V_DIST
    constexpr static bool VALIDATION_LAYERS_ENABLED = true;
#else
    constexpr static bool VALIDATION_LAYERS_ENABLED = false;
#endif
};

} // venture
