#include "Serializer/Serializer.h"
#include "Scene/Scene.h"
#include <filesystem>
#include <fstream>

#include "EditorComponents/EditorComponents.h"

void Canella::Serializer::Serialize(std::weak_ptr<Scene> scene, const std::string &scenes_path,const std::string &assets_path)
{
    if (const auto sceneRef = scene.lock())
    {
        m_ProjectFolder = scenes_path;
        const auto project_path = std::filesystem::absolute(scenes_path);
        for (const auto &file : std::filesystem::directory_iterator(project_path))
        {
            std::string file_name = file.path().filename().string();
            std::string extension = file.path().extension().string();
            if (file_name == "scene.json")
                LoadEntities(sceneRef, scenes_path+ "//" + file_name);
        }
    }

    LoadMaterialData(scene,assets_path);

}

void Canella::Serializer::LoadMaterialData( std::weak_ptr<Scene> scene,const std::string &projectPath ) {

    const auto project_path = std::filesystem::absolute(projectPath);
    auto filepath = project_path.string() +"\\" + "Materials";

    for(const auto& p: std::filesystem::recursive_directory_iterator(filepath)) {
        auto file = p.path().string();
        auto scene_locked = scene.lock();
        std::fstream   f( file );
        nlohmann::json material_meta;
        f >> material_meta;

         MaterialDescription material;
         material.pipeline = material_meta["Pipeline"];
         material.name = material_meta["Name"];

         for(auto& texture_slot : material_meta["TextureSlots"])
         {
             TextureSlot slot;
             slot.texture_source = texture_slot["TextureSource"];
             slot.semantic = texture_slot["SemanticSlot"];
             material.texture_slots.push_back( slot);
         }
         scene_locked->material_library[material_meta["Name"]] = material;
    }
}


void Canella::Serializer::Deserialize(std::weak_ptr<Scene> scene)
{
    if (const auto scene_ref = scene.lock())
        DeserializeEntities(scene_ref, m_ProjectFolder+ "\\" + "scene.json");
}

void Canella::Serializer::LoadEntities(std::shared_ptr<Scene> scene, const std::string &filepath)
{
    std::fstream f(filepath);
    nlohmann::json scene_data;
    f >> scene_data;
    scene_data = scene_data["Scene"];
    for (auto entity_meta : scene_data["Entities"])
    {
        auto uuid  = entity_meta["UUID"].get<std::uint64_t>();
        //todo fix this weird if statement
        Entity& created_entity = scene->CreateEntity(uuid);
        created_entity.uuid = uuid;
        created_entity.name = entity_meta["Name"].get<std::string>();
        LoadComponents(scene, created_entity.raw_id(), entity_meta["Components"]);
    }

    resolve_references(scene);
}

void Canella::Serializer::LoadComponents(
    std::shared_ptr<Scene> scene,
    const entt::entity entity,
    nlohmann::json &components_data)
{
    for (auto component_data : components_data)
    {
        const std::string type = component_data["type"].get<std::string>();
        if(type == "Transform")
            SerializeTransform( component_data, scene, entity);
        else if(type == "Camera")
            SerializeCamera( component_data, scene->registry, entity);
        else if(type == "MeshAsset")
            SerializeMeshAsset( component_data, scene, entity);
        else if( type == "CameraEditor")
            SerializeCameraEditor( component_data, scene->registry, entity);
    }
}

/**
 * \brief
 * \param scene
 * \param scene_data
 */
void Canella::Serializer::DeserializeEntities(
    const std::shared_ptr<Scene> scene,
    const std::string& scene_data)
{
    std::ofstream o(scene_data);
    nlohmann::json out_put;
    out_put["Scene"]["Entities"] =  nlohmann::json::array();
    nlohmann::json &entities =  out_put["Scene"]["Entities"] ;
    
    auto iterator = scene->entityLibrary.begin();
    while(iterator != scene->entityLibrary.end())
    {
        if(iterator->second->name == "Scene_Root")
        {
            ++iterator;
            continue;
        }
        nlohmann::json entity;
        entity["Name"] = iterator->second->name;
        entity["UUID"] = iterator->second->uuid;
        entity["Components"] = nlohmann::json::array();
        nlohmann::json& components = entity["Components"];
        
        auto view = scene->registry.view<TransformComponent,CameraComponent,ModelAssetComponent,Behavior>();
        if(iterator->second->has_component<CameraComponent>())
        {
            nlohmann::json component;
            DeserializeCamera(component,view.get<CameraComponent>(iterator->first));
            components.push_back(component);
        }

        if(iterator->second->has_component<TransformComponent>())
        {
            nlohmann::json component;
            DeserializeTransform(component,view.get<TransformComponent>(iterator->first));
            components.push_back(component);
        }

        if(iterator->second->has_component<ModelAssetComponent>())
        {
            nlohmann::json component;
            DeserializeMeshAsset(component,view.get<ModelAssetComponent>(iterator->first));
            components.push_back(component);
        }

        if(iterator->second->has_component<Behavior>())
        {
            nlohmann::json component;
            auto behavior =  view.get<Behavior>(iterator->first);
            auto  camera_editor = (CameraEditor*)behavior.instance;

            DeserializeCameraEditor(component,*camera_editor);
            components.push_back(component);
        }
        entity["Parent"] = "Root";
        entities.push_back(entity);
        ++iterator;
    }
    o<< std::setw(4)<<out_put.dump();
    o.close();
}

void Canella::Serializer::resolve_references( std::shared_ptr<Scene> scene ) {

    auto iterator = scene->entityLibrary.begin();
    while(iterator != scene->entityLibrary.end())
    {
        //Root  has no parent... so just skip it
        if(iterator->second->name == "Scene_Root")
        {
            ++iterator;
            continue;
        }

        auto entity = iterator->second;
        auto view = scene->registry.view<TransformComponent,CameraComponent,ModelAssetComponent,Behavior>();
        auto& transform_component = entity->get_component<TransformComponent>();
        ResolveReferencesInTransform(transform_component,scene);
        ++iterator;
    }

}


