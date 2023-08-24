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

    hierarchy();

    if(selected_entity.lock())
        build_property_window(ImVec2(min_width,0));
    ImGui::End();

}

void Canella::Inspector::hierarchy() {
    float item_height = ImGui::GetTextLineHeightWithSpacing();
    if (ImGui::BeginChildFrame(ImGui::GetID("Hierarchy"),ImVec2(-FLT_MIN, 10 * item_height))) {

        auto       &scene = application->scene;
        auto       i      = 0;
        for ( auto &entity: scene->entityLibrary ) {
            auto id = static_cast<uint32_t>(entity.first);
            ImGui::PushID( id );
            auto       row          = "entity " + std::to_string( id );
            static int node_clicked = -1;
            auto       node_flags   = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                      ImGuiTreeNodeFlags_SpanAvailWidth;
            if ( selected_entity.lock())
                if ( selected_entity.lock()->raw_id() == entity.second->raw_id())
                    node_flags |= ImGuiTreeNodeFlags_Selected;
            bool node_open = ImGui::TreeNodeEx(( void * ) ( intptr_t ) i, node_flags, entity.second->name.c_str());
            if ( ImGui::IsItemClicked()) {
                selected_entity = entity.second;
                node_clicked    = i;
            }
            create_entity_popup();
            if ( node_open ) {
                inspect_chilren( entity.second );
                ImGui::TreePop();
            }
            ImGui::Separator();
            ImGui::PopID();
            i++;
        }
        ImGui::EndChildFrame();
    }
}

void Canella::Inspector::create_entity_popup() {
    if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
    {
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        if (ImGui::Button("Create"))
        {
            application->scene->CreateEntity();
        }
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
        ImGui::Text("Components");
        auto selected = selected_entity.lock();
        display_transform(selected);
        ImGui::EndChildFrame();
    }
}

void Canella::Inspector::inspect_chilren( std::shared_ptr<Entity> parent ) {

}

