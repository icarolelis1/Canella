#pragma once
#ifndef RENDER
#define RENDER
#include <json.hpp>
#include <iostream>
#include <stb.h>
#include "Instance/Instance.h"
#include "Device/Device.h"
namespace RenderSystem{
    
    struct RenderConfig
    {
        const char* API = "VULKAN";
    };
    
    class Render{
        public:
            Render(nlohmann::json& json);
            static  RenderSystem::Render* createRender( nlohmann::json &configFile);
            RenderConfig& getConfig();
            static RenderSystem::Render* render_instance;
        private:
            RenderConfig config;
            stbi_uc* pixels;
            void initializeBackend();
    };
}
#endif