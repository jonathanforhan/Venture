cmake_minimum_required(VERSION 3.26)
project(Venture)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++20")
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(extern)
add_subdirectory(src)

target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        glfw
        glm
)
