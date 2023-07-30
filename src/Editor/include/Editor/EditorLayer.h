#ifndef CANELLA_EDITORLAYER_H
#define CANELLA_EDITORLAYER_H
#include "Eventsystem/Eventsystem.hpp"
#include "Entity.h"

namespace Canella
{
    class OnSelectEntity : public Event<std::weak_ptr<Entity>>
    {
    };
    class OnDeselect : public Event<>
    {
    };

    class EditorLayer
    {
    public:
        EditorLayer() = default;
         void setup_layer(OnSelectEntity& on_select_entity_event);
        ~EditorLayer() = default;
        void draw_layer();
    private:
        bool entity_changed = false;
        void action_on_select_entity(std::weak_ptr<Entity> entity);
        void action_on_deselect_entity();
        std::weak_ptr<Entity> selected_entity;
    };
}


#endif //CANELLA_EDITORLAYER_H
