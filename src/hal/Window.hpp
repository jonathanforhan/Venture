#pragma once

#include "vulkan/VulkanWindow.hpp"

namespace venture {

#ifdef V_VULKAN_RENDERER
using Window = vulkan::VulkanWindow;
#endif

} // venture
