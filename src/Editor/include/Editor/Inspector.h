#pragma once
#ifndef CANELLA_INSPECTOR_H
#define CANELLA_INSPECTOR_H
#include "imgui.h"
#include "Project/Project.h"

namespace Canella

{
    class OnSelectEntity : public Event<std::weak_ptr<Entity>>{};
    class OnDeselect : public Event<>{};

    class Inspector
    {
    public :
        Inspector();
        void build();
        void set_application(Application* applicaiton);
        std::weak_ptr<Canella::Entity> selected_entity;
        OnSelectEntity on_select_entity;

    private:
        Application* application;
        void build_property_window(ImVec2 window_offset);
        void save_button();
        void create_entity_popup();
        void inspect_chilren( std::shared_ptr<Entity> shared_ptr_1 );
        void hierarchy();
        void setup_deselection_event();
    };
}

#endif //CANELLA_INSPECTOR_H
