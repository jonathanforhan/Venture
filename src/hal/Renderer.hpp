#pragma once

#include "vulkan/VulkanRenderer.hpp"

namespace venture {

#ifdef V_VULKAN_RENDERER
using Renderer = vulkan::VulkanRenderer;
#endif

} // venture
