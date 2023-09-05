cmake_minimum_required(VERSION 3.26)
project(Hydrogen VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" COMMAND python pre_build.py)

find_package(Vulkan REQUIRED)

add_subdirectory(extern)
add_subdirectory(src)

target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        glfw
        cglm
)

set(H_DEBUG TRUE)

if (H_DIST)
    message("Distribution Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DH_DIST=1)
elseif (H_RELEASE)
    message("Release Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DH_RELEASE=1)
elseif (H_DEBUG)
    message("Debug Build")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DH_DEBUG=1)
else ()
    message(FATAL_ERROR "Invalid build config! options are '-DH_DIST | -DH_RELEASE | -DH_DIST'")
endif ()

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DH_VULKAN=1)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        -DH_LOGGING_ENABLED=1
        -DH_LOG_INFO=1
        -DH_LOG_WARNINGS=1
        -DH_LOG_ERRORS=1
        -DH_LOG_VERBOSE=1
)

file(COPY spirv DESTINATION ${CMAKE_BINARY_DIR})
