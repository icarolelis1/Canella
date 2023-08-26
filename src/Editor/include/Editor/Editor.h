#ifndef CANELLA_EDITOR
#define CANELLA_EDITOR
#pragma once
#include <iostream>
#include <fstream>
#include <json.hpp>
#include "Project/Project.h"
#include "Logger/Logger.hpp"
#include <Eventsystem/Eventsystem.hpp>
#include "EditorLayer.h"
#include <memory>
#define BASE_CONFIG_FILE "resources\\config\\config.json"
#define USE_BASE_SAMPLE 0

#define USE_COLOR_STYLE_1 1

/*
 * #bdbdbd	(189,189,189)
#32393d	(50,57,61)
#373737	(55,55,55)
#2d2d2d	(45,45,45)
#464646	(70,70,70)
 */
#if USE_COLOR_STYLE_1
#define MAIN_BG ImColor(45,45,45)
#define TITLE_BG ImColor(55,55,55)
#define BORDER ImColor(50,57,61)
#define FONT_COLOR ImColor(189,189,189)
#define MENU_BG ImColor(70,70,70)
#define BLUE ImColor(54, 79, 107)
#endif

namespace Canella
{

/*    Canella::Entity &Canella::Mouse::instance()
    {
        static Mouse mouse;
        return mouse;
    }*/



    class Editor;

    class StartPlayEvent : public Event<Canella::Editor &>
    {
    };
    class StopPlayEvent : public Event<Canella::Editor &>
    {
    };
    class Editor
    {
    public:
        Editor();
        Editor(const Editor &) = delete;
        ~Editor();
        void run_editor();
        void play();
        void stop();
        std::weak_ptr<Entity> selected_entity;

    private:

        Canella::EditorLayer layer;
        StartPlayEvent OnStartPlay;
        StopPlayEvent OnStopPlayEvent;
        std::unique_ptr<Canella::Application> application;
        Canella::RenderSystem::VulkanBackend::VulkanRender render;
        VkDescriptorPool imguiPool;

        void render_editor_gui(VkCommandBuffer &, uint32_t);
        void setup_imgui();
        void display_graphics_status();
        void bind_shortcuts();
        bool game_mode = true;
        //Display Window
        bool show_status;
        bool show_inspector;

        //Editor Events
        OnSelectEntity& on_select_entity;
        OnOutputStatsEvent out_put_stats;
        OnSelectOperation on_select_operation;
        OnDeselect on_deselect;
    };
}

#endif