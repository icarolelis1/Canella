#ifndef CANELLA_EDITORLAYER_H
#define CANELLA_EDITORLAYER_H
#include <ImGuizmo.h>
#include "Eventsystem/Eventsystem.hpp"
#include "Entity.h"

namespace Canella
{

    class OnSelectEntity : public Event<std::weak_ptr<Entity>>{};
    class OnSelectOperation : public Event<IMGUIZMO_NAMESPACE::OPERATION>{};
    class OnDeselect : public Event<>{};

    class EditorLayer
    {
    public:
        EditorLayer() = default;
         void setup_layer(Canella::OnSelectEntity &on_select_entity_event,Canella::OnSelectOperation& on_select_operation );
        ~EditorLayer() = default;
        void draw_layer();
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
