#pragma once
#ifndef WINDOW
#define WINDOW
#include <iostream>
#include "Input/Input.h"

#include <vulkan/vulkan.h>
#include <json.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include "Windowing.h"

namespace Canella
{
    class GlfwWindow : public Windowing
    {
    
    public:

        GlfwWindow() = default;
        ~GlfwWindow();
        GLFWwindow* getHandle();
        void initialize(nlohmann::json& config);
        GLFWwindow* m_window;
        int shouldCloseWindow();
        Extent& getExtent();
        void getSurface(VkInstance instance, VkSurfaceKHR* surface);
        void update();

    private:

        Mouse mouse;
        KeyBoard keyboard;

    };
}

#endif