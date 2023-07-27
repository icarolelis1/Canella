#include "Components/Components.h"
#include "EditorComponents/EditorComponents.h"

void Canella::SerializeTransform(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    const auto view = registry.view<TransformComponent>();
    auto &[position,
           rotation,
           scale,
           model_matrix,
           children,
           parent] =
        view.get<TransformComponent>(entity);

    position.x = data["Position"]["x"].get<float>();
    position.y = data["Position"]["y"].get<float>();
    position.z = data["Position"]["z"].get<float>();

    scale.x = data["Scale"]["x"].get<float>();
    scale.y = data["Scale"]["y"].get<float>();
    scale.z = data["Scale"]["z"].get<float>();

    rotation.x = data["Rotation"]["x"].get<float>();
    rotation.y = data["Rotation"]["y"].get<float>();
    rotation.z = data["Rotation"]["z"].get<float>();
}

void Canella::SerializeCamera(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    auto &camera_component = registry.emplace<CameraComponent>(entity);
    auto pos_x = data["Position"]["x"].get<float>();
    auto pos_y = data["Position"]["y"].get<float>();
    auto pos_z = data["Position"]["z"].get<float>();

    auto yaw  = data["Yaw"].get<float>();
    auto pitch = data["Pitch"].get<float>();
    auto roll = data["Roll"].get<float>();

    camera_component.position = glm::vec3(pos_x, pos_y, pos_z);
    camera_component.yaw = yaw;
    camera_component.pitch = pitch;
    camera_component.roll = roll;
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
    model.model_matrix = &view_transform.get<TransformComponent>(entity).modelMatrix;
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
}

void Canella::DeserializeCamera(nlohmann::json &data, CameraComponent &camera_component)
{
    data["type"] = "Camera";
}

void Canella::DeserializeMeshAsset(nlohmann::json &data, ModelAssetComponent &mesh_asset_component)
{
    data["type"] = "MeshAsset";
}

void Canella::DeserializeCameraEditor(nlohmann::json &data, ModelAssetComponent &camera_editor_component)
{
    data["type"] = "CameraEditor";
}