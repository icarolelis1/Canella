#include "Editor/Inspector.h"
#include "Components/Components.h"
#include "imgui_internal.h"


//Property View Components
void display_transform(std::shared_ptr<Canella::Entity> entity)
{
    auto& transform = entity->get_component<Canella::TransformComponent>();
    ImGui::Text("Transform");
    ImGui::Separator();
    ImGui::Text("Position");
    ImGui::DragFloat( "X ###PosX", &transform.position.x );
    ImGui::DragFloat( "Y ###PosY", &transform.position.y );
    ImGui::DragFloat( "Z ###PosZ", &transform.position.z );
    ImGui::Separator();
    ImGui::Text("Rotation");
    ImGui::DragFloat( "X ###RotX", &transform.rotation.x );
    ImGui::DragFloat( "Y ###RotY", &transform.rotation.y );
    ImGui::DragFloat( "Z ###RotZ", &transform.rotation.z );
    ImGui::Separator();
    ImGui::Text("Scale");
    ImGui::DragFloat( "X ###ScaleX", &transform.scale.x );
    ImGui::DragFloat( "Y ###ScaleY", &transform.scale.y );
    ImGui::DragFloat( "Z ###ScaleZ", &transform.scale.z );
}

void Canella::Inspector::build() {

    if(application == nullptr) return;
    bool             open         = true;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background
    if ( !ImGui::Begin( "Inspector", &open,window_flags)) {
        ImGui::End();
        return;
    }
    save_button();
    ImGuiIO &io            = ImGui::GetIO();
    float   display_width  = ( float ) io.DisplaySize.x;
    float   display_height = ( float ) io.DisplaySize.y;
    ImGui::SetWindowPos( ImVec2( 0, 0 ));
    auto min_width = glm::min( display_width * 0.25f, 520.0f );
    ImGui::SetWindowSize( ImVec2( min_width, display_height ));
    ImGui::Dummy(ImVec2(0,5));

    hierarchy_tab();

    if(selected_entity.lock())
        build_property_window(ImVec2(min_width,0));
    ImGui::End();

}

void Canella::Inspector::hierarchy_tab() {

    float item_height = ImGui::GetTextLineHeightWithSpacing();
    if (ImGui::BeginChildFrame(ImGui::GetID("Hierarchy"),ImVec2(-FLT_MIN, 10 * item_height))) {

        auto       &scene = application->scene;
        //Transform component of the root
        auto& root_transform =  scene->root->get_component<Canella::TransformComponent>();
        //Root Entity Raw_id
        auto raw_id = root_transform.owner->raw_id();
        //Grabbing the Root Entity using its raw Id;
        auto entity = scene->entityLibrary.find(raw_id)->second;

        auto id = static_cast<uint32_t>(entity->uuid);
        ImGui::PushID(id);
        auto node_flags   = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |ImGuiTreeNodeFlags_SpanAvailWidth;
        if ( selected_entity.lock())
            if ( selected_entity.lock()->raw_id() == entity->raw_id())
                node_flags |= ImGuiTreeNodeFlags_Selected;

        auto i = 0;
        bool node_open = ImGui::TreeNodeEx(( void * ) ( intptr_t )i, node_flags, entity->name.c_str());

        if ( ImGui::IsItemClicked()) {
            selected_entity = entity;
            on_select_entity.invoke(selected_entity);
        }

        std::pair<entt::entity,Entity*> pair = std::make_pair(entity->raw_id(),entity.get());
        drag_and_drop_behavior( scene, pair);
        create_entity_popup();
        if ( node_open ) {hierarchy_entity(entity);ImGui::TreePop(); }

        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::EndChildFrame();
}

void Canella::Inspector::hierarchy_entity(std::shared_ptr<Entity>  entity) {

    auto& transform_parent = entity->get_component<Canella::TransformComponent>();
    auto  i  = 0;
    auto& scene = application->scene;
    for ( auto& transform: transform_parent.children ) {
        auto &entity = *transform->owner;
        auto shared_ptr_entity = scene->entityLibrary.find(transform->owner->raw_id())->second;
        auto id = static_cast<uint32_t>(entity.uuid);
        ImGui::PushID( id );
        auto node_flags   = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |ImGuiTreeNodeFlags_SpanAvailWidth;
        if ( selected_entity.lock())
            if ( selected_entity.lock()->raw_id() == entity.raw_id())
                node_flags |= ImGuiTreeNodeFlags_Selected;

        bool node_open = ImGui::TreeNodeEx(( void * ) ( intptr_t ) i, node_flags, entity.name.c_str());
        if ( ImGui::IsItemClicked()) {
            selected_entity = shared_ptr_entity;
            on_select_entity.invoke(selected_entity);
        }

        std::pair<entt::entity,Entity*> pair = std::make_pair(entity.raw_id(),&entity);
        drag_and_drop_behavior( scene, pair);
        create_entity_popup();
        if ( node_open ) {
            hierarchy_entity(shared_ptr_entity);
            ImGui::TreePop();
        }

        ImGui::Separator();
        ImGui::PopID();
        i++;
    }
}


void Canella::Inspector::drag_and_drop_behavior( const std::shared_ptr<Canella::Scene> &scene,
                                                 const std::pair<const entt::entity,  Entity*> entity) const {
    if(ImGui::BeginDragDropSource())
    {
        if(entity.second->name != "Scene_Root")
        {
            ImGui::SetDragDropPayload("Entity", &entity.first, sizeof(entity.first));
            ImGui::Text(entity.second->name.c_str());
        }
        ImGui::EndDragDropSource();
    }

    if(ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(entt::entity));
            entt::entity payload_n = *(entt::entity*)payload->Data;
            auto entity_dragged = scene->entityLibrary[payload_n];
            auto&  transform = entity_dragged->get_component<Canella::TransformComponent>();

            //Remove from previous parent
            transform.parent->children.remove(&transform);
            auto& target_transform = entity.second->get_component<TransformComponent>();
            transform.parent = &target_transform;
            target_transform.children.push_back(&transform);

        }
        ImGui::EndDragDropTarget();
    }
}

void Canella::Inspector::create_entity_popup() {
    if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
    {
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        if (ImGui::Button("Create"))
            application->scene->create_root_parented_entity();
        ImGui::EndPopup();
    }
}

void Canella::Inspector::save_button() {
    if(ImGui::Button( "Save"))
        ImGui::OpenPopup("Save?");

    if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            application->Deserialize();
        }
        ImGui::Dummy(ImVec2(20.f,0.f));
        if (ImGui::Button("No", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void Canella::Inspector::set_application( Canella::Application *application ) {
    this->application = application;
}

void Canella::Inspector::build_property_window(ImVec2 window_offset) {
    ImGui::Dummy(ImVec2(0,25));
    float item_height = ImGui::GetTextLineHeightWithSpacing();
    if (ImGui::BeginChildFrame(ImGui::GetID("frame"),ImVec2(-FLT_MIN, 10 * item_height)))
    {

        ImGui::InputText("Name ", selected_entity.lock()->name.data(), IM_ARRAYSIZE(selected_entity.lock()->name.data()));
        ImGui::Text("Components");
        auto selected = selected_entity.lock();
        display_transform(selected);
        ImGui::EndChildFrame();
    }
}


void Canella::Inspector::setup_deselection_event() {
    auto &mouse = Mouse::instance();
    std::function<void(MouseButton, InputAction)> mouse_press = [=](MouseButton button, InputAction action) -> void
    {
        if (button == MouseButton::RIGHT_MOUSE)
        {

            if (action == InputAction::RELEASE)
            {
                selected_entity.reset();
                on_select_entity.invoke(selected_entity);
            }
        }
    };
    Event_Handler<MouseButton, InputAction> handler_click(mouse_press);
    mouse.OnMouseClick += handler_click;
}

Canella::Inspector::Inspector() {
    //setup_deselection_event();
}


