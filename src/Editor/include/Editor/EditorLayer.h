#ifndef CANELLA_EDITORLAYER_H
#define CANELLA_EDITORLAYER_H
#include <ImGuizmo.h>
#include "Eventsystem/Eventsystem.hpp"
#include "Entity.h"
#include "Project/Project.h"
#include "Editor/Inspector.h"

namespace Canella
{
    class OnSelectOperation : public Event<IMGUIZMO_NAMESPACE::OPERATION>{};
    class EditorLayer
    {
    public:
        EditorLayer();
         void setup_layer(Application *application, OnSelectOperation& on_select_operation );
        ~EditorLayer() = default;
        void draw_guizmos();
        OnSelectEntity& on_select_entity;
        Canella::Inspector inspector;
    private:
        IMGUIZMO_NAMESPACE::OPERATION operation = ImGuizmo::TRANSLATE;
        bool entity_changed = false;
        void action_on_select_entity(std::weak_ptr<Entity> entity);
        void action_select_operation(IMGUIZMO_NAMESPACE::OPERATION operation);
        void action_on_deselect_entity();
        std::weak_ptr<Entity> selected_entity;
    };
}


#endif //CANELLA_EDITORLAYER_H
