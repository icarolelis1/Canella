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
    /**
     * brief  User Project
     */
    class Project
    {

    public:
        Project() = default;
        void load(nlohmann::json &config);
        void run();
        void close();
        ~Project();
        std::string assetsFolder;
    private:
        void init_systems();
        void update_systems();
        void setup_project_folder(nlohmann::json& data);
        
        CameraComponent main_camera;
        entt::registry registry;
        bool playing = true;
        std::unique_ptr<Canella::Render> render;
        std::shared_ptr<Canella::Scene> scene;
        Canella::GlfwWindow window;
        Canella::Serializer serializer;
    };

}

#endif