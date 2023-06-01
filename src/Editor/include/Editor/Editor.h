#ifndef CANELLA_EDITOR
#define CANELLA_EDITOR
#pragma once
#include <iostream>
#include <fstream>
#include <json.hpp>
#include "Project/Project.h"
#include "Logger/Logger.hpp"
#include <Eventsystem/Eventsystem.hpp>

#define BASE_CONFIG_FILE "resources\\config\\config.json"
#define USE_BASE_SAMPLE 0
namespace Canella{
    class Editor;

    class StartPlayEvent: public Event<Canella::Editor&>{};
    class StopPlayEvent: public Event<Canella::Editor&>{};


    class Editor{
    public:

        Editor();
        Editor(const Editor&) = delete;
        ~Editor();
        void run_editor();
        void play();
        void stop();

    private:
        StartPlayEvent OnStartPlay;
        StopPlayEvent OnStopPlayEvent;
        Canella::Application application;
        Canella::RenderSystem::VulkanBackend::VulkanRender render;
        Canella::GlfwWindow window;
        VkDescriptorPool imguiPool;
        void setup_imgui();
        void render_editor_gui(VkCommandBuffer& ,uint32_t,RenderSystem::VulkanBackend::FrameData&);
        bool edit_mode = true;

    };
}


#endif