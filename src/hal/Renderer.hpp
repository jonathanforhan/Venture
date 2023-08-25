#pragma once

#include "vulkan/VulkanRenderer.hpp"

namespace venture {

#ifdef VENTURE_VULKAN_RENDERER
using Renderer = vulkan::VulkanRenderer;
#endif

} // venture
