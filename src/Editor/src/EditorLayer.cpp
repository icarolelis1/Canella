#include "Editor/EditorLayer.h"
#include "Window/Window.h"
#include "Components/Components.h"
#include <glm/gtx/matrix_decompose.hpp>

bool decompose_model(glm::mat4& transform,glm::vec3& translation, glm::vec3& rotation,glm::vec3& scale)
{
    //from glm::decompose
    //Extracted from glm and modified by the Cherno https://www.youtube.com/watch?v=Pegb5CZuibU&t=655s&ab_channel=TheCherno
    using namespace glm;
    mat4 local_matrix(transform);

    if( epsilonEqual(local_matrix[3][3],static_cast<float>(0),epsilon<float>()))
        return false;

    if(( epsilonNotEqual(local_matrix[0][3],static_cast<float>(0), epsilon<float>()))||
       ( epsilonNotEqual(local_matrix[1][3],static_cast<float>(0), epsilon<float>()))||
       ( epsilonNotEqual(local_matrix[2][3],static_cast<float>(0), epsilon<float>())))
    {
        local_matrix[0][3] = local_matrix[1][3]  = local_matrix[2][3] = static_cast<float>(0);
        local_matrix[3][3] = static_cast<float>(1);
    }

    translation = vec3(local_matrix[3]);
    local_matrix[3] = vec4(0,0,0,local_matrix[3].w);

    vec3 Row[3],pDum[3];

    for( length_t i = 0; i < 3 ; ++i)
        for( length_t j = 0; j < 3; ++j)
            Row[i][j] = local_matrix[i][j];

    scale.x = length(Row[0]);
    Row[0] = detail::scale(Row[0],static_cast<float>(1));
    scale.y = length(Row[1]);
    Row[1] = detail::scale(Row[1],static_cast<float>(1));
    scale.z = length(Row[2]);
    Row[2] = detail::scale(Row[2],static_cast<float>(1));

    rotation.y = asin(-Row[0][2]);
    if(cos(rotation.y) != 0)
    {
        rotation.x = atan2(Row[1][2],Row[2][2]);
        rotation.z = atan2(Row[0][1],Row[0][0]);
    }
    else
    {
        rotation.x = atan2(-Row[2][0],Row[1][1]);
        rotation.z = 0;
    }
    return true;
}

void Canella::EditorLayer::setup_layer( Canella::OnSelectEntity &on_select_entity_event,Canella::OnSelectOperation& on_select_operation )
{
    std::function<void(std::weak_ptr<Entity>)> on_select = [=](std::weak_ptr<Entity> entity){ action_on_select_entity(entity); };
    std::function<void(IMGUIZMO_NAMESPACE::OPERATION)> on_select_oepration = [=](IMGUIZMO_NAMESPACE::OPERATION operation){ action_select_operation(operation); };
    on_select_entity_event+=on_select;
    on_select_operation += on_select_oepration;
}


void Canella::EditorLayer::draw_layer() {

    if (entity_changed)
    {
        auto& window = GlfwWindow::get_instance();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect( 0,0,window.getExtent().width,window.getExtent().height);
        ImGuizmo::Enable(true);

        auto& camera_projection = selected_entity.lock()->get_owner_scene().lock()->main_camera->projection;
        auto& camera_view = selected_entity.lock()->get_owner_scene().lock()->main_camera->view;
        auto& entity_transform = selected_entity.lock()->get_component<TransformComponent>();
        float tmpMatrix[16];
        ImGuizmo::RecomposeMatrixFromComponents(&entity_transform.position.x, &entity_transform.rotation.x, &entity_transform.scale.x, tmpMatrix);
        ImGuizmo::Manipulate(&camera_view[0][0], &camera_projection[0][0], operation,
                             ImGuizmo::MODE::WORLD, tmpMatrix);

        if (ImGuizmo::IsUsing())
        {
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(tmpMatrix, matrixTranslation, matrixRotation, matrixScale);
            switch (operation)
            {
                case ImGuizmo::OPERATION::TRANSLATE:
                    entity_transform.position = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
                    break;

                case ImGuizmo::OPERATION::ROTATE:
                    entity_transform.rotation = glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
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
}

void Canella::EditorLayer::action_select_operation(IMGUIZMO_NAMESPACE::OPERATION op)
{
    operation = op;
}

void Canella::EditorLayer::action_on_deselect_entity() {
    entity_changed = false;
}

