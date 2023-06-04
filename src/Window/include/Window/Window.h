#pragma once
#ifndef WINDOW
#define WINDOW
#include <iostream>
#include "Input/Input.h"

#include <vulkan/vulkan.h>
#include <json.hpp>
#define GLFW_INCLUDE_VULKAN
#ifdef DWIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Windowing.h"

namespace Canella
{
    class WidowResizeEvent: public Event<Extent>{};
    class WidowFocusEvent: public Event<>{};

    class GlfwWindow : public Windowing
    {
    
    public:
        static GlfwWindow * get_instance();
        void initialize(nlohmann::json& config);
        GlfwWindow(GlfwWindow&) = delete;
        void operator=(const GlfwWindow&) = delete;
        ~GlfwWindow();
        void getSurface(VkInstance instance, VkSurfaceKHR* surface);
        void set_title_data() override;
        int shouldCloseWindow();
        void wait_idle();
        void update();

        //Singleton
        static GlfwWindow* instance;
        //Window Events
        WidowResizeEvent OnWindowResize;
        WidowFocusEvent OnWindowFocus;

        Extent getExtent();
        GLFWwindow* m_window;
        GLFWwindow* getHandle();

    private:
        GlfwWindow() = default;
        std::string title;
        Mouse mouse;
        KeyBoard keyboard;

    };
}

#endif