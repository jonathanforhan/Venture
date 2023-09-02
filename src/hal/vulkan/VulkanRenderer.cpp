#include "VulkanRenderer.hpp"
#include <fstream>
#include <iostream>
#include <ranges>
#include <set>
#include "error_handling/Check.hpp"
#include "error_handling/Log.hpp"
#include "Debug.hpp"

namespace venture::vulkan {

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
          _swapchain_images(),
          _swapchain_framebuffers(),
          _command_pool(),
          _command_buffers(),
          _render_pass(),
          _pipeline_layout(),
          _graphics_pipeline()
{
    try {
        create_instance();
        create_surface();
        retrieve_physical_device();
        create_logical_device();
        create_swapchain();
        create_render_pass();
        create_graphics_pipeline();
        create_framebuffers();
        create_graphics_command_pool();
        create_command_buffers();
        record_commands();
    } catch (const std::exception &e) {
        log(Error, e.what());
        throw e; // unwind stack and crash program
    }
}

void VulkanRenderer::draw()
{
	// TODO
}

void VulkanRenderer::create_instance()
{
	if constexpr (VALIDATION_LAYERS_ENABLED)
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
    check_DEBUG(verify_instance_extension_support({glfw_exts, glfw_ext_count}));

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

    void *ici_next                   = VALIDATION_LAYERS_ENABLED ? &debug_create_info       : nullptr;
    size_t ici_enabled_layer_count   = VALIDATION_LAYERS_ENABLED ? VALIDATION_LAYERS.size() : 0U;
    auto *ici_enabled_layer_names    = VALIDATION_LAYERS_ENABLED ? VALIDATION_LAYERS.data() : nullptr;

    vk::InstanceCreateInfo instance_create_info = {
			.sType = vk::StructureType::eInstanceCreateInfo,
			.pNext = ici_next,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = static_cast<uint32_t>(ici_enabled_layer_count),
			.ppEnabledLayerNames = ici_enabled_layer_names,
			.enabledExtensionCount = glfw_ext_count,
			.ppEnabledExtensionNames = glfw_exts,
    };
    _instance = vk::createInstanceUnique(instance_create_info);
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
        vk::DeviceQueueCreateInfo device_queue_create_info = {
				.sType = vk::StructureType::eDeviceQueueCreateInfo,
				.queueFamilyIndex = static_cast<uint32_t>(index),
				.queueCount = 1,
				.pQueuePriorities = &priority,
        };
        
        dev_queue_create_info_collection.emplace_back(device_queue_create_info);
    }

    vk::DeviceCreateInfo device_create_info = {
            .sType = vk::StructureType::eDeviceCreateInfo,
            .queueCreateInfoCount = static_cast<uint32_t>(dev_queue_create_info_collection.size()),
            .pQueueCreateInfos = dev_queue_create_info_collection.data(),
            .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
            .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data()
    };

    _logical_device = _physical_device.createDeviceUnique(device_create_info);

    _logical_device->getQueue(_queue_family_info.graphics_family_index, 0, &_graphics_queue);
    _logical_device->getQueue(_queue_family_info.presentation_family_index, 0, &_presentation_queue);
}

void VulkanRenderer::create_swapchain()
{
    _swapchain_info = SwapchainInfo::get_info(_physical_device, *_surface, _window);

    uint32_t sci_image_count = _swapchain_info.surface_capabilities.minImageCount + 1;
    if (_swapchain_info.surface_capabilities.maxImageCount != 0)
    {
        sci_image_count = std::min(sci_image_count, _swapchain_info.surface_capabilities.maxImageCount);
    }

    uint32_t queue_family_array[] = {
            (uint32_t) _queue_family_info.graphics_family_index,
            (uint32_t) _queue_family_info.presentation_family_index
    };

    bool same_queue = _queue_family_info.graphics_family_index == _queue_family_info.presentation_family_index;

    auto sci_image_sharing_mode       = same_queue ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    auto sci_queue_family_index_count = same_queue ? uint32_t(0)                 : uint32_t(2);
    auto *sci_queue_family_indices    = same_queue ? nullptr                     : queue_family_array;

    vk::SwapchainCreateInfoKHR swapchain_create_info {
			.sType = vk::StructureType::eSwapchainCreateInfoKHR,
			.surface = *_surface,
			.minImageCount = sci_image_count,
			.imageFormat = _swapchain_info.surface_format.format,
			.imageColorSpace = _swapchain_info.surface_format.colorSpace,
			.imageExtent = _swapchain_info.extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = sci_image_sharing_mode,
			.queueFamilyIndexCount = sci_queue_family_index_count,
			.pQueueFamilyIndices = sci_queue_family_indices,
			.preTransform = _swapchain_info.surface_capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = _swapchain_info.present_mode,
			.clipped = true,
			.oldSwapchain = nullptr
    };

    _swapchain = _logical_device->createSwapchainKHRUnique(swapchain_create_info);

    auto images = _logical_device->getSwapchainImagesKHR(*_swapchain);
	for (const auto& image : images)
	{
		auto img_view = make_image_view(image, _swapchain_info.surface_format.format, vk::ImageAspectFlagBits::eColor);
        _swapchain_images.emplace_back(image, std::move(img_view));
    }
}

void VulkanRenderer::create_render_pass()
{
    vk::AttachmentDescription color_attachment_desc = {
            .format = _swapchain_info.surface_format.format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };

    vk::AttachmentReference color_attachment_ref = {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    vk::SubpassDescription subpass_desc = {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
    };

    //--- define where our subpass can change image layout state
    vk::SubpassDependency subpass_dependencies[2];
    // vk::ImageLayout::eUndefined => vk::ImageLayout::eColorAttachmentOptimal
    subpass_dependencies[0] = vk::SubpassDependency {
			.srcSubpass = vk::SubpassExternal,
			.dstSubpass = 0,
			.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe,
			.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.srcAccessMask = vk::AccessFlagBits::eMemoryRead,
			.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
    };
    // vk::ImageLayout::eColorAttachmentOptimal => vk::ImageLayout::ePresentSrcKHR,
    subpass_dependencies[1] = vk::SubpassDependency {
			.srcSubpass = 0,
			.dstSubpass = vk::SubpassExternal,
			.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe,
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eMemoryRead,
    };

    vk::RenderPassCreateInfo render_pass_create_info = {
			.sType = vk::StructureType::eRenderPassCreateInfo,
			.attachmentCount = 1,
			.pAttachments = &color_attachment_desc,
			.subpassCount = 1,
			.pSubpasses = &subpass_desc,
			.dependencyCount = sizeof subpass_dependencies / sizeof *subpass_dependencies,
			.pDependencies = subpass_dependencies,
    };

    _render_pass = _logical_device->createRenderPassUnique(render_pass_create_info);
}

void VulkanRenderer::create_graphics_pipeline()
{
    //--- Shaders
    vk::UniqueShaderModule vert_mod = make_shader_module(VERT_PATH);
    vk::UniqueShaderModule frag_mod = make_shader_module(FRAG_PATH);

    vk::PipelineShaderStageCreateInfo vert_shader_create_info = {
            .sType = vk::StructureType::ePipelineShaderStageCreateInfo,
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vert_mod,
            .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo frag_shader_create_info = {
            .sType = vk::StructureType::ePipelineShaderStageCreateInfo,
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *frag_mod,
            .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo shader_stages[] = {
            vert_shader_create_info,
            frag_shader_create_info
    };

    //--- Vertex Input
    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
            .sType = vk::StructureType::ePipelineVertexInputStateCreateInfo,
            // TODO
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
    };

    //--- Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
            .sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo,
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = false,
    };

    //--- Viewport and Scissor
    vk::Viewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(_swapchain_info.extent.width),
            .height = static_cast<float>(_swapchain_info.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };

    vk::Rect2D scissor = {
            .offset = {0, 0},
            .extent = _swapchain_info.extent
    };

    vk::PipelineViewportStateCreateInfo viewport_state_create_info = {
            .sType = vk::StructureType::ePipelineViewportStateCreateInfo,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
    };

    //--- Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info = {
            .sType = vk::StructureType::ePipelineRasterizationStateCreateInfo,
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = false,
            .lineWidth = 1.0f,
    };

    //--- Multisampling
    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info = {
            .sType = vk::StructureType::ePipelineMultisampleStateCreateInfo,
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = false,
    };

    //--- Depth Stencil
    // TODO

    //--- Blending
    // equation : (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
    vk::PipelineColorBlendAttachmentState color_blend_attachment_state = {
            .blendEnable = true,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info = {
            .sType = vk::StructureType::ePipelineColorBlendStateCreateInfo,
            .logicOpEnable = false,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment_state,
    };

    //--- Dynamic States
    /*
    vk::DynamicState dynamic_states[] = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info = {
            .sType = vk::StructureType::ePipelineDynamicStateCreateInfo,
            .dynamicStateCount = sizeof(dynamic_states) / sizeof(vk::DynamicState),
            .pDynamicStates = dynamic_states,
    };
    */

    //--- Pipeline Layout
	vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {
			.sType = vk::StructureType::ePipelineLayoutCreateInfo,
			// TODO
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
	};

    _pipeline_layout = _logical_device->createPipelineLayoutUnique(pipeline_layout_create_info);


    //--- Graphics Pipeline
    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info = {
            .sType = vk::StructureType::eGraphicsPipelineCreateInfo,
            .stageCount = sizeof shader_stages / sizeof *shader_stages,
            .pStages = shader_stages,
            .pVertexInputState = &vertex_input_state_create_info,
            .pInputAssemblyState = &input_assembly_state_create_info,
            .pViewportState = &viewport_state_create_info,
            .pRasterizationState = &rasterization_state_create_info,
            .pMultisampleState = &multisample_state_create_info,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = nullptr,
            .layout = *_pipeline_layout,
            .renderPass = *_render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };

    auto [result, value] = _logical_device->createGraphicsPipelineUnique(VK_NULL_HANDLE, graphics_pipeline_create_info);
    check(result == vk::Result::eSuccess);
    _graphics_pipeline = std::move(value);
}

void VulkanRenderer::create_framebuffers()
{
    _swapchain_framebuffers.resize(_swapchain_images.size());

    for (auto i : std::views::iota(0UL, _swapchain_images.size()))
    {
		std::array<vk::ImageView, 1> attachments = {
				*_swapchain_images[i].image_view
		};

		vk::FramebufferCreateInfo frame_buffer_create_info = {
				.sType = vk::StructureType::eFramebufferCreateInfo,
				.renderPass = *_render_pass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = _swapchain_info.extent.width,
				.height = _swapchain_info.extent.height,
				.layers = 1,
		};

		_swapchain_framebuffers[i] = _logical_device->createFramebufferUnique(frame_buffer_create_info);
	}
}

void VulkanRenderer::create_graphics_command_pool()
{   
	vk::CommandPoolCreateInfo command_pool_create_info = {
			.sType = vk::StructureType::eCommandPoolCreateInfo,
			.queueFamilyIndex = static_cast<uint32_t>(_queue_family_info.graphics_family_index),
	};

	_command_pool = _logical_device->createCommandPoolUnique(command_pool_create_info);
}

void VulkanRenderer::create_command_buffers()
{
    _command_buffers.resize(_swapchain_framebuffers.size());

    vk::CommandBufferAllocateInfo command_buffer_alloc_info = {
			.sType = vk::StructureType::eCommandBufferAllocateInfo,
			.commandPool = *_command_pool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = static_cast<uint32_t>(_command_buffers.size()),
    };

    _command_buffers = _logical_device->allocateCommandBuffersUnique(command_buffer_alloc_info);
}

void VulkanRenderer::record_commands()
{
    vk::CommandBufferBeginInfo command_buffer_begin_info = {
            .sType = vk::StructureType::eCommandBufferBeginInfo,
            .flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse,
    };

    vk::ClearValue clear_values[] = {
            vk::ClearColorValue(0.5f, 0.6f, 0.4f, 1.0f)
    };

    vk::RenderPassBeginInfo render_pass_begin_info = {
            .sType = vk::StructureType::eRenderPassBeginInfo,
            .renderPass = *_render_pass,
            .renderArea = {
                    .offset = { 0, 0 },
                    .extent = _swapchain_info.extent,
            },
            .clearValueCount = sizeof clear_values / sizeof *clear_values,
            .pClearValues = clear_values,
    };

    for (size_t i = 0; i < _command_buffers.size(); i++)
    {
        _command_buffers[i]->begin(command_buffer_begin_info);

        render_pass_begin_info.framebuffer = *_swapchain_framebuffers[i];
		_command_buffers[i]->beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        // Render Pass
			_command_buffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, *_graphics_pipeline);
			_command_buffers[i]->draw(3, 1, 0, 0);

		_command_buffers[i]->endRenderPass();

        _command_buffers[i]->end();
    }
}

vk::UniqueImageView
VulkanRenderer::make_image_view(vk::Image image, vk::Format format, vk::ImageAspectFlagBits flags) const
{
    vk::ImageViewCreateInfo image_view_create_info = {
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
    };
    return _logical_device->createImageViewUnique(image_view_create_info);
}

vk::UniqueShaderModule VulkanRenderer::make_shader_module(const char *path) const
{
    std::ifstream ifs(path, std::ios::binary); // raii
    check_DEBUG(ifs.is_open());
    auto code = std::vector<char>(std::istreambuf_iterator<char>(ifs), {});

    vk::ShaderModuleCreateInfo shader_module_create_info = {
            .sType = vk::StructureType::eShaderModuleCreateInfo,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };

    return _logical_device->createShaderModuleUnique(shader_module_create_info);
}

void VulkanRenderer::retrieve_physical_device()
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
        auto match_ext = [=](vk::ExtensionProperties prop) -> bool {
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

    for (const auto &dev_ext : DEVICE_EXTENSIONS)
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

    for (const char *validation_layer : VALIDATION_LAYERS)
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

bool VulkanRenderer::verify_physical_device_suitable(vk::PhysicalDevice physical_device) const
{
    bool queue_family_valid = QueueFamilyInfo::get_info(physical_device, *_surface).is_valid();
    bool swap_chain_valid = SwapchainInfo::get_info(physical_device, *_surface, _window).is_valid();
    bool dev_ext_support = verify_device_extension_support(physical_device);

    return queue_family_valid && swap_chain_valid && dev_ext_support;
}

} // venture::vulkan
