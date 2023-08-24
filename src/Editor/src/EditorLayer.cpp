#include "Editor/EditorLayer.h"
#include "Window/Window.h"
#include "Components/Components.h"
#include "Editor/Inspector.h"
#include "Editor/EditorLayer.h"

void Canella::EditorLayer::setup_layer(Canella::Application* application, Canella::OnSelectEntity &on_select_entity_event,Canella::OnSelectOperation& on_select_operation )
{
    inspector.set_application(application);
    std::function<void(std::weak_ptr<Entity>)> on_select = [=](std::weak_ptr<Entity> entity){ action_on_select_entity(entity); };
    std::function<void(IMGUIZMO_NAMESPACE::OPERATION)> on_select_oepration = [=](IMGUIZMO_NAMESPACE::OPERATION operation){ action_select_operation(operation); };
    on_select_entity_event+=on_select;
    on_select_operation += on_select_oepration;
}

void Canella::EditorLayer::draw_layer() {

    inspector.build();
    if (entity_changed)
    {
        auto& window = GlfwWindow::get_instance();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect( 0,0,window.getExtent().width,window.getExtent().height);
        ImGuizmo::Enable(true);

        auto camera_projection =  selected_entity.lock()->get_owner_scene().lock()->main_camera->projection;
        camera_projection[1][1] *=-1;
        auto camera_view = selected_entity.lock()->get_owner_scene().lock()->main_camera->view;
        auto& entity_transform = selected_entity.lock()->get_component<TransformComponent>();
        float tmpMatrix[16];
     /*   camera_view[1][1] *=-1;
        camera_view[2][2] *=-1;*/

        //Dont use euler angles to manipulate my orientation
        glm::vec3 nul = glm::vec3(0.0f,0.0f,0.0f);
        ImGuizmo::RecomposeMatrixFromComponents(&entity_transform.position.x, &nul.x, &entity_transform.scale.x, tmpMatrix);
        ImGuizmo::Manipulate(&camera_view[0][0], &camera_projection[0][0], operation,
                             ImGuizmo::MODE::LOCAL, tmpMatrix);

        if (ImGuizmo::IsUsing())
        {
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(tmpMatrix, matrixTranslation, matrixRotation, matrixScale);
            auto delta =  -glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
            auto quat = glm::quat(glm::radians(delta));

            switch (operation)
            {
                case ImGuizmo::OPERATION::TRANSLATE:
                    entity_transform.position = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
                    break;

                case ImGuizmo::OPERATION::ROTATE:
                    entity_transform.orientation *= quat;
                    break;

                case ImGuizmo::OPERATION::SCALE:
                    entity_transform.scale = glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]);
                    break;
                default:
                    break;
            }
        }
    }

}

void Canella::EditorLayer::action_on_select_entity(std::weak_ptr<Entity> entity) {

    selected_entity = entity;
    entity_changed = true;
    inspector.selected_entity =  entity;
}

void Canella::EditorLayer::action_select_operation(IMGUIZMO_NAMESPACE::OPERATION op)
{
    operation = op;
}

void Canella::EditorLayer::action_on_deselect_entity() {
    entity_changed = false;
}

