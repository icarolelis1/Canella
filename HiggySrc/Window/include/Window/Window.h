#pragma once
#ifndef WINDOW
#define WINDOW
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <json.hpp>
#include "Input/Input.h"
#include <iostream>
namespace Engine{
    class Window{
        public:
            Window();
            void initialize(nlohmann::json& config);
        private:
            GLFWwindow *m_window;
             Mouse * mouse;
            KeyBoard * keyboard;

    };

}


#endif