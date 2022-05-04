#pragma once
#ifndef WINDOW
#define WINDOW
#include <GLFW/glfw3.h>
#include <json.hpp>
#include "Input/Input.h"
#include <iostream>


namespace Engine
{
    class Window
    {

    public:
        Window();
        void initialize(nlohmann::json &config);
        ~Window();
        GLFWwindow *m_window;
        int shouldCloseWindow();
      

    private:
        Mouse *mouse;
        KeyBoard *keyboard;
    };
}

#endif