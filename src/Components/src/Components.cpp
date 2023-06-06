#include "Components/Components.h"
#include "EditorComponents/EditorComponents.h"

void Canella::SerializeTransform(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    registry.emplace<TransformComponent>(entity);
    const auto view = registry.view<TransformComponent>();
    auto & [position, rotation, scale,model_matrix] =
            view.get<TransformComponent>(entity);
    position.x = data["Position"]["x"].get<float>();
    position.y = data["Position"]["y"].get<float>();
    position.z = data["Position"]["z"].get<float>();
}

//todo separate the  serialization of cameraComponent and CameraEditorController
void Canella::SerializeCamera(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    auto& camera_component = registry.emplace<CameraComponent>(entity);
}

void Canella::SerializeMeshAsset(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    registry.emplace<ModelAssetComponent>(entity);
    const auto view = registry.view<ModelAssetComponent>();
    auto& [model, source,isStatic] =  view.get<ModelAssetComponent>(entity);
    source = data["Source"].get<std::string>();
    isStatic = data["Static"].get<bool>();

    //Pass the model matrix pointer to mesh comming from TransformComponent
    const auto view_transform = registry.view<TransformComponent>();
    model.model_matrix   = &view_transform.get<TransformComponent>(entity).modelMatrix;
}

void Canella::SerializeCameraEditor(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    auto& camera_editor_controller = registry.emplace<Behavior>(entity);
    camera_editor_controller.bind<CameraEditor>();
    const auto view = registry.view<CameraComponent>();
    ((CameraEditor*)camera_editor_controller.instance)->camera_component = &view.get<CameraComponent>(entity);
}

void Canella::DeserializeTransform(nlohmann::json& data,TransformComponent& transform_component)
{
    data["type"] = "Transform";
    data["Position"]["x"] = transform_component.position.x;
    data["Position"]["y"] = transform_component.position.y;
    data["Position"]["z"] = transform_component.position.z;
}

void Canella::DeserializeCamera(nlohmann::json& data,CameraComponent& camera_component)
{
    data["type"] = "Camera";
}

void Canella::DeserializeMeshAsset(nlohmann::json& data,ModelAssetComponent& mesh_asset_component)
{
    data["type"] = "MeshAsset";
}

void Canella::DeserializeCameraEditor(nlohmann::json& data,ModelAssetComponent& camera_editor_component)
{
    data["type"] = "CameraEditor";
}