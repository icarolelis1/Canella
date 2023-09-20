#pragma once
#ifndef APPLICATION
#define APPLICATION
#include "Entity.h"
#include "Serializer/Serializer.h"
#include "Render/Render.h"
#include "MaterialManager/MaterialManager.h"
#include "Window/Window.h"
#include "Threadpool/Threadpool.h"
#include "Logger/Logger.hpp"
#include "JobSystem/JobSystem.h"
#include "Scene/Scene.h"
#include <iostream>
#include <json.hpp>
#include <iostream>
#include <string>

namespace Canella
{
    class Application
    {

    public:
        Application(Canella::GlfwWindow*,Canella::Render*);
        ~Application();
        void load(nlohmann::json &config);
        void run();
        std::string assets_folder;
        std::shared_ptr<Canella::Scene> scene;
        void Deserialize();
    private:
        Canella::MaterialCollection material_collection;
        void setup_project_folder(nlohmann::json& data);
        Time application_time;
        double frame_time;
        entt::registry registry;
        bool playing = true;
        Canella::Render* render;
        Canella::GlfwWindow* window;
        Canella::Serializer serializer;
        friend class Editor;
    };

}

#endif