#include "Serializer/Serializer.h"
#include "Scene/Scene.h"
#include <filesystem>
#include <fstream>

#include "EditorComponents/EditorComponents.h"

Canella::Serializer::Serializer(const std::string& project): m_ProjectFolder(project) {}


void Canella::Serializer::Serialize(std::weak_ptr<Scene> scene, const std::string &projectPath)
{
    if (const auto sceneRef = scene.lock())
    {
        m_ProjectFolder = projectPath;
        const auto project_path = std::filesystem::absolute(projectPath);
        for (const auto &file : std::filesystem::directory_iterator(project_path))
        {
            std::string file_name = file.path().filename().string();
            std::string extension = file.path().extension().string();
            if (file_name == "scene.json")
                LoadEntities(sceneRef, projectPath+ "//" + file_name);
        }
    }
    else
        Logger::Error("Invalid scene for serialization");
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
        if(uuid == -1)
        {
            Entity& created_entity = scene->CreateEntity();
            created_entity.uuid = uuid;
            created_entity.name = entity_meta["Name"].get<std::string>();
            LoadComponents(scene, created_entity.raw_id(), entity_meta["Components"]);
        }
        else
        {
            Entity& created_entity = scene->CreateEntity(uuid);
            created_entity.uuid = uuid;
            created_entity.name = entity_meta["Name"].get<std::string>();
            LoadComponents(scene, created_entity.raw_id(), entity_meta["Components"]);
        }
    }

    //Resolve References
    for (auto& entity : scene->entityLibrary)
    {
        
    }
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
            SerializeMeshAsset( component_data, scene->registry, entity);
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

