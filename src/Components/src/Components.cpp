#include "Components/Components.h"

void Canella::SerializeTransform(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    registry.emplace<TransformComponent>(entity);
    const auto view = registry.view<TransformComponent>();
    auto & [position, rotation, scale,model_matrix] = view.get<TransformComponent>(entity);
    position.x = data["Position"]["x"].get<float>();
    position.y = data["Position"]["y"].get<float>();
    position.z = data["Position"]["z"].get<float>();
}

void Canella::SerializeCamera(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    registry.emplace<CameraComponent>(entity);
}

void Canella::SerializeMeshAsset(nlohmann::json& data,entt::registry& registry,entt::entity entity)
{
    registry.emplace<MeshAssetComponent>(entity);
    const auto view = registry.view<MeshAssetComponent>();
    auto& [mesh, source,isStatic] =  view.get<MeshAssetComponent>(entity);
    source = data["Source"].get<std::string>();
    isStatic = data["Static"].get<bool>();
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

void Canella::DeserializeMeshAsset(nlohmann::json& data,MeshAssetComponent& mesh_asset_component)
{
    data["type"] = "MeshAsset";
}