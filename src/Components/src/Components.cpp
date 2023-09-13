#include "Components/Components.h"
#include "EditorComponents/EditorComponents.h"
#include "CanellaUtility/CanellaUtility.h"


/*
glm::vec4 compute_sphere_bounding_volume(Canella::Mesh &mesh, std::vector<Canella::Vertex> &vertices)
{
    std::vector<Canella::Vertex> slice = std::vector<Canella::Vertex>(vertices.begin() + mesh.vertex_offset, vertices.begin() + mesh.vertex_count + mesh.vertex_offset);

    glm::vec3 center = glm::vec3(0);

    for (auto &vertex : slice)
        center += glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
    center /= slice.size();

    double radius = 0.0f;
    for (auto &v : slice)
        radius = max(radius, glm::distance(center, glm::vec3(v.position.x, v.position.y, v.position.z)));

    return glm::vec4(center.x, center.y, center.z, radius);
}
*/



void Canella::SerializeTransform( nlohmann::json &data, std::shared_ptr<Scene> scene, entt::entity entity)
{
    const auto view = scene->registry.view<TransformComponent>();
    auto &[owner,position,rotation,
           orientation,scale,
           model,
           reference,
           children,
           parent] =  view.get<TransformComponent>(entity);

    owner = scene->entityLibrary[entity].get();
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

    reference.uid = data["References"][0]["Entity"].get<std::uint64_t>();
    if(reference.uid != 0)
        scene->entityLibrary[entity]->is_dirty = true;


}

void Canella::SerializeCamera(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    auto &camera_component = registry.emplace<CameraComponent>(entity);
    camera_component.entity_transform = &registry.get<TransformComponent>(entity);
    camera_component.yaw = data["Yaw"].get<float>();
    camera_component.pitch = data["Pitch"].get<float>();

}

void Canella::SerializeMeshAsset(nlohmann::json &data, entt::registry &registry, entt::entity entity)
{
    registry.emplace<ModelAssetComponent>(entity);
    const auto view = registry.view<ModelAssetComponent>();
    auto &[model,source,instance_src,material_name, isStatic,instance_count] = view.get<ModelAssetComponent>(entity);
    source = data["Source"].get<std::string>();
    instance_src = data["InstanceData"].get<std::string>();
    isStatic = data["Static"].get<bool>();
    model.instance_count = data["InstanceCount"].get<std::int32_t>();


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


    data["References"][0]["Type"] = stringify(ComponentType::Transform);
    data["References"][0]["Entity"] =  transform_component.parent->owner->uuid;



}

void Canella::DeserializeCamera(nlohmann::json &data, CameraComponent &camera_component)
{
    data["type"] = "Camera";
    data["Fovy"] = camera_component.fovy;
    data["Znear"] = camera_component.zNear;
    data["Zfar"] = camera_component.zFar;
    data["Yaw"] = camera_component.yaw;
    data["Pitch"] = camera_component.pitch;
    data["UID"] = 0;
}

void Canella::DeserializeMeshAsset(nlohmann::json &data, ModelAssetComponent &mesh_asset_component)
{
    data["type"] = "MeshAsset";
    data["Source"] = mesh_asset_component.source;
    data["InstanceCount"] =1;
    data["Static"] =  mesh_asset_component.isStatic;
    data["InstanceData"] =  mesh_asset_component.instance_src;
    data["UID"] = 0;

}

void Canella::DeserializeCameraEditor(nlohmann::json &data, CameraEditor &camera_editor_component)
{
    data["type"] = "CameraEditor";
    data["UID"] = 0;
}

void Canella::ResolveReferencesInTransform( Canella::TransformComponent &transform_component,
                                            std::shared_ptr<Scene> scene ) {
    if(transform_component.reference.uid == 0)
    {
        auto& root_transform = scene->root->get_component<TransformComponent>();
        transform_component.parent = &root_transform;

        root_transform.children.push_back(&transform_component);
        return;
    }
    auto& referenced_entity = scene->get_entity_by_uuid(transform_component.reference.uid);
    auto& referenced_transform = referenced_entity.get_component<TransformComponent>();
    transform_component.parent  = &referenced_transform;
    referenced_transform.children.push_back(&transform_component);
}

/*
void Canella::resolve_component_references(std::shared_ptr<Scene> scene,std::vector<Reference> &references ) {
    for(auto& ref : references)
    {
        Canella::assign_reference(scene,ref);
    }
}

void Canella::assign_reference( std::shared_ptr<Scene> scene, Canella::Reference reference ) {
    auto uid = reference.uid;
    //TODO FIX USE THE UID AS KEY TO THE MAP OF ENTITIES AND NOT LOOP THROUGH EVERYTHING
    for(auto& [entt,entity] : scene->entityLibrary)
        if(entity->uuid == uid)
        {
            switch ( reference.type ) {
                case ComponentType::Transform :
                    reference.uid = entity->uuid;
                    break;
            }
        }
};

*/
