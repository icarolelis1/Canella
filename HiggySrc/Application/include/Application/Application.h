#pragma once
#ifndef APPLICATION
#define APPLICATION
#include <json.hpp>
#include <iostream>
#include <string>


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



    };

}



#endif