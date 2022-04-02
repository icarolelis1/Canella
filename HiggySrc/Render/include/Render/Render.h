#pragma once
#ifndef RENDER
#define RENDER
#include <json.hpp>
#include "vulkan/vulkan.h"
#include <iostream>
#include <stb.h>
namespace RenderSystem{

    struct RenderConfig
    {
        const char* API = "VULKAN";
    };
    
    class Render{
        public:
            Render();
           static  RenderSystem::Render* createRender( nlohmann::json &configFile);
            RenderConfig& getConfig();
            static RenderSystem::Render* render_instance;
        private:
            RenderConfig config;
            stbi_uc* pixels;
    };

}
#endif