#pragma once
#ifndef APPLICATION
#define APPLICATION
#include <json.hpp>
#include <iostream>
#include <string>
#include "Entity/Entity.h"
#include "Render/Render.h"
#include "Component/Component.h"
#include "Window/Window.h"
#include "Scenegraph/Scenegraph.h"
#include <iostream>
#include "Threadpool/Threadpool.h"
#include "Logger/Logger.hpp"


namespace Application{
    class App{
        public:
            App();
            void initialize(nlohmann::json& config);
            void run();
            void close();
            ~App();
        

        private:
            //Application metadata;
            std::string appName;
            std::string AppVersion;
            std::string graphicsAPI;
            RenderSystem::Render* render;
            Engine::Window window;
            std::unique_ptr<Engine::Scenegraph> scenegraph;
            std::unique_ptr<Engine::Threadpool> threadPool;
            int playing = 1;


    };

}



#endif