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
namespace Application
{
    class App
    {

    public:
        App() = default;
        void initialize(nlohmann::json &config);
        void run();
        void close();
        ~App();
        std::string& assetsFolder;
    private:
        void SetupProjectFolder(nlohmann::json& data);
        entt::registry registry;
        bool playing = true;
        std::unique_ptr<Canella::Render> render;
        std::shared_ptr<Canella::Scene> scene;
        Canella::GlfwWindow window;
        Canella::Serializer serializer;
    };

}

#endif