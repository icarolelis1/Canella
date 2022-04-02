#pragma once
#ifndef APPLICATION
#define APPLICATION
#include <json.hpp>
#include <iostream>
#include <string>
#include <Entity/Entity.h>
#include <Render/Render.h>
#include <Component/Component.h>
#include <Window/Window.h>

namespace Application{
    class App{
        public:
            App();
            void initialize(nlohmann::json& config);
        private:
            //Application metadata;
            std::string appName;
            std::string AppVersion;
            std::string graphicsAPI;
            RenderSystem::Render* render;
            Engine::Window window;


    };

}



#endif