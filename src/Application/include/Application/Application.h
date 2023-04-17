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
#include <iostream>
#include "Threadpool/Threadpool.h"
#include "Logger/Logger.hpp"


namespace Application{
    class App{
        public:
            App() = default;
            void initialize(nlohmann::json& config);
            void run();
            void close();
            ~App();
        

        private:
            //Application metadata;
            bool playing = true;
            std::unique_ptr<Canella::Render> render;
            Canella::GlfwWindow window;


    };

}



#endif