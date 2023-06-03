#include "Serializer/Serializer.h"
#include "Scene/Scene.h"
#include <filesystem>
#include <fstream>


Canella::Serializer::Serializer(const std::string& project): m_ProjectFolder(project) {}

/**
 * \brief Serialize the scene to a file
 * \param scene
 */
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
        Entity created_entity = scene->CreateEntity();
        LoadComponents(scene, created_entity.raw_id(), entity_meta["Components"]);
    }
}

/**
 * \brief Builds the components for each entity in the config File
 * \param entity entity the component is associated to
 * \param components_data Data Serialized for each component
 */
void Canella::Serializer::LoadComponents(
    std::shared_ptr<Scene> scene,
    const entt::entity entity,
    nlohmann::json &components_data)
{
    for (auto component_data : components_data)
    {
        const std::string type = component_data["type"].get<std::string>();
        if(type == "Transform")
            SerializeTransform(component_data,scene->m_registry,entity);
        else if(type == "Camera")
            SerializeCamera(component_data,scene->m_registry,entity);
        else if(type == "MeshAsset")
            SerializeMeshAsset(component_data,scene->m_registry,entity);
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
    
    auto iterator = scene->m_EntityLibrary.begin();
    while(iterator != scene->m_EntityLibrary.end())
    {
        nlohmann::json entity;
        entity["Components"] = nlohmann::json::array();
        nlohmann::json& components = entity["Components"];
        
        auto view = scene->m_registry.view<TransformComponent,CameraComponent,ModelAssetComponent>();
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
        
        entity["id"] = (iterator->first);
        entities.push_back(entity);
        ++iterator;
    }
    o<< std::setw(4)<<out_put.dump();
    o.close();

}
