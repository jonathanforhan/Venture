cmake_minimum_required(VERSION 3.26)
project(Venture VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND python3 compile_shaders.py)

find_package(Vulkan REQUIRED)

add_subdirectory(extern)
add_subdirectory(src)

target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        glfw
        glm
)

if (V_DIST)
    message("Distribution Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DV_DIST=1)
elseif (V_RELEASE)
    message("Release Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DV_RELEASE=1)
elseif (V_DEBUG)
    message("Debug Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DV_DEBUG=1)
else ()
    message(FATAL_ERROR "Invalid build config! options are '-DV_DIST | -DV_RELEASE | -DV_DIST'")
endif ()

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DV_VULKAN_RENDERER=1)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        -DV_LOGGING_ENABLED=1
        -DV_LOG_INFO=1
        -DV_LOG_WARNINGS=1
        -DV_LOG_ERRORS=1
)

file(COPY spirv DESTINATION ${CMAKE_BINARY_DIR})
