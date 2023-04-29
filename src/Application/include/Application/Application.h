#pragma once
#ifndef APPLICATION
#define APPLICATION
#include "Entity/Entity.h"
#include "Render/Render.h"
#include "Component/Component.h"
#include "Window/Window.h"
#include "Threadpool/Threadpool.h"
#include "Logger/Logger.hpp"
#include "ComponentRegistry/ComponentRegistry.h"
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

    private:
        // Application metadata;
        bool playing = true;
        std::unique_ptr<Canella::Render> render;
        Canella::Scene scene;
        Canella::GlfwWindow window;
    };

}

#endif