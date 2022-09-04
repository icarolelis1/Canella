#pragma once
#ifndef RENDER
#define RENDER
#include <json.hpp>
#include <iostream>
#include <stb.h>
#include "Windowing.h"

namespace Canella{
    
    struct RenderConfig
    {
        const char* API = "VULKAN";
    };
        
    class Render{
        public:
            Render() = default;
            Render(nlohmann::json& json);
            Render(RenderConfig& config);

            virtual void initialize(Windowing* window) = 0;

            virtual void render() = 0;

            virtual void update(float time) = 0;

            RenderConfig& getConfig();

        protected:
            RenderConfig config;
    };
}
#endif