#include "Debug.hpp"
#include <iostream>

namespace venture::vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data
)
{
    (void)message_type;
    (void)user_data;
    std::cout << "Vulkan API Debug ";
    switch (message_severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            std::cout << "Info -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            std::cout << "Verbose -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cout << "Warning -- ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cerr << "Error";
            std::cout << " -- ";
            break;
        default:
            std::cout << "Unknown -- ";
    }
    std::cout << callback_data->pMessage << '\n';
    return false;
}

} // venture::vulkan
