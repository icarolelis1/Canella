#pragma once
#ifndef RENDER
#define RENDER
#include <json.hpp>
#include "vulkan/vulkan.h"
namespace RenderSystem{

    struct RenderConfig
    {
        const char* API = "VULKAN";
    };
    
    class Render{
        public:
            Render();
            static Render createRender( nlohmann::json &configFile);
            RenderConfig& getConfig();
        private:
            RenderConfig config;
    };

}
#endif