#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <ranges>
#include <iostream>
#include <vector>

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);

    vk::Instance instance;
    uint32_t extensionCount = 0;
    vk::Result result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    assert(result == vk::Result::eSuccess);
    std::cout << "Extension Count: " << extensionCount << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
