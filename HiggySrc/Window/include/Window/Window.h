#pragma once
#ifndef WINDOW
#define WINDOW
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <json.hpp>
namespace Engine{
    class Window{
        public:
            Window();
            void initialize(nlohmann::json& config);
    };

}


#endif