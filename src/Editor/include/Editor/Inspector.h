#pragma once
#ifndef CANELLA_INSPECTOR_H
#define CANELLA_INSPECTOR_H
#include "imgui.h"
#include "Project/Project.h"

namespace Canella
{
    class Inspector
    {
    public :
        Inspector() = default;
        void build();
        void set_application(Application* applicaiton);
        std::weak_ptr<Canella::Entity> selected_entity;

    private:
        Application* application;
        void build_property_window(ImVec2 window_offset);

        void save_button();

        void create_entity_popup();

        void inspect_chilren( std::shared_ptr<Entity> shared_ptr_1 );

        void hierarchy();
    };
}

#endif //CANELLA_INSPECTOR_H
