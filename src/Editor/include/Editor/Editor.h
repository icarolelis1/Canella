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

#define USE_COLOR_STYLE_1  1

#if USE_COLOR_STYLE_1
#define MAIN_BG  ImColor(251, 243, 212)
#define TITLE_BG ImColor(92, 44, 12)
#define BORDER   ImColor(130, 140, 148)
#define FONT_COLOR ImColor(130, 140, 148)
#define MENU_BG ImColor(251, 243, 212)

#endif


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
        std::unique_ptr<Canella::Application> application;
        Canella::RenderSystem::VulkanBackend::VulkanRender render;
        VkDescriptorPool imguiPool;
        std::vector<Canella::TimeQueryData*> time_queries;
        bool custom_font_pushed = false;
        void setup_imgui();
        void render_editor_gui(VkCommandBuffer& ,uint32_t,RenderSystem::VulkanBackend::FrameData&);
        bool edit_mode = true;

        void editor_layout();
    };
}


#endif