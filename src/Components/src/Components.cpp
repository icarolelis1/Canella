#include "Components/Components.h"
#include "EditorComponents/EditorComponents.h"

void Canella::SerializeTransform( nlohmann::json &data, std::shared_ptr<Scene> scene, entt::entity entity)
{
    const auto view = scene->registry.view<TransformComponent>();
    auto &[position,rotation,orientation,scale,model,parent] =
        view.get<TransformComponent>(entity);

    position.x = data["Position"]["x"].get<float>();
    position.y = data["Position"]["y"].get<float>();
    position.z = data["Position"]["z"].get<float>();
    scale.x    = data["Scale"]["x"].get<float>();
    scale.y    = data["Scale"]["y"].get<float>();
    scale.z    = data["Scale"]["z"].get<float>();

    rotation.x = data["Rotation"]["x"].get<float>();
    rotation.y = data["Rotation"]["y"].get<float>();
    rotation.z = data["Rotation"]["z"].get<float>();

    orientation.x = data["Orientation"]["x"].get<float>();
    orientation.y = data["Orientation"]["y"].get<float>();
    orientation.z = data["Orientation"]["z"].get<float>();
    orientation.w = data["Orientation"]["w"].get<float>();
}

void Canella::SerializeCamera(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    auto &camera_component = registry.emplace<CameraComponent>(entity);
    camera_component.entity_transform = &registry.get<TransformComponent>(entity);

}

void Canella::SerializeMeshAsset(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    registry.emplace<ModelAssetComponent>(entity);
    const auto view = registry.view<ModelAssetComponent>();
    auto &[model, source, isStatic,instance_count] = view.get<ModelAssetComponent>(entity);
    source = data["Source"].get<std::string>();
    isStatic = data["Static"].get<bool>();
    model.instance_count = data["InstanceCount"].get<std::uint32_t>();

    // Pass the model matrix pointer to mesh comming from TransformComponent
    const auto view_transform = registry.view<TransformComponent>();
    model.model_matrix = &view_transform.get<TransformComponent>(entity).model_matrix;
}

void Canella::SerializeCameraEditor(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    auto &camera_editor_controller = registry.emplace<Behavior>(entity);
    camera_editor_controller.bind<CameraEditor>();
    const auto view = registry.view<CameraComponent>();
    ((CameraEditor *)camera_editor_controller.instance)->camera_component = &view.get<CameraComponent>(entity);
}

void Canella::DeserializeTransform(nlohmann::json &data, TransformComponent &transform_component)
{
    data["type"] = "Transform";
    data["Position"]["x"] = transform_component.position.x;
    data["Position"]["y"] = transform_component.position.y;
    data["Position"]["z"] = transform_component.position.z;

    data["Rotation"]["x"] = transform_component.rotation.x;
    data["Rotation"]["y"] = transform_component.rotation.y;
    data["Rotation"]["z"] = transform_component.rotation.z;

    data["Scale"]["x"] = transform_component.scale.x;
    data["Scale"]["y"] = transform_component.scale.y;
    data["Scale"]["z"] = transform_component.scale.z;

    data["Orientation"]["x"] = transform_component.orientation.x;
    data["Orientation"]["y"] = transform_component.orientation.y;
    data["Orientation"]["z"] = transform_component.orientation.z;
    data["Orientation"]["w"] = transform_component.orientation.w;
}

void Canella::DeserializeCamera(nlohmann::json &data, CameraComponent &camera_component)
{
    data["type"] = "Camera";
    data["Fovy"] = camera_component.fovy;
    data["Znear"] = camera_component.zNear;
    data["Zfar"] = camera_component.zFar;
}

void Canella::DeserializeMeshAsset(nlohmann::json &data, ModelAssetComponent &mesh_asset_component)
{
    data["type"] = "MeshAsset";
    data["Source"] = mesh_asset_component.source;
    data["InstanceCount"] = 1 ;
    data["Static"] =  mesh_asset_component.isStatic;

}

void Canella::DeserializeCameraEditor(nlohmann::json &data, CameraEditor &camera_editor_component)
{
    data["type"] = "CameraEditor";
}