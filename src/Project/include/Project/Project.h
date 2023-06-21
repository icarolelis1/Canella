#pragma once
#ifndef APPLICATION
#define APPLICATION
#include "Entity.h"
#include "Serializer/Serializer.h"
#include "Render/Render.h"
#include "Window/Window.h"
#include "Threadpool/Threadpool.h"
#include "Logger/Logger.hpp"
#include "VulkanRender/VulkanRender.h"
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
        void close();
        std::string assets_folder;
        std::shared_ptr<Canella::Scene> scene;

    private:
        void init_systems();
        void update_systems(float frame_time);
        void setup_project_folder(nlohmann::json& data);
        Time application_time;
        float frame_time;
        entt::registry registry;
        bool playing = true;

        Canella::Render* render;
        Canella::GlfwWindow* window;
        Canella::Serializer serializer;
    };

}

#endif