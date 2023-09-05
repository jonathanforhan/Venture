set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS    OFF)
add_subdirectory(glfw)

set(CGLM_SHARED OFF)
set(CGLM_STATIC ON)
add_subdirectory(cglm EXCLUDE_FROM_ALL)