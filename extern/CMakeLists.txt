set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS    OFF)
add_subdirectory(glfw)
add_subdirectory(glm)

if (BUILD_VULKAN_HPP)
    message("Building Vulkan-hpp")
    add_subdirectory(vulkan-hpp)
endif ()
