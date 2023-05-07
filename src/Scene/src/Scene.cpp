#include "Scene/Scene.h"
#include "Entity.h"
/**
 * \brief brief Create a new Entity in this scene
 * \return The Entity created
 */
Canella::Entity Canella::Scene::CreateEntity()
{
    entt::entity entt_entity = m_registry.create();
    Entity entity{entt_entity, this->shared_from_this() };
    m_EntityLibrary[entt_entity] = entity;
    return m_EntityLibrary[entt_entity];
}

/**
 * \brief Builds the components for each entity in the config File
 * \param entity entity the component is associated to
 * \param components_data Data Serialized for each component
 */

/*void Canella::Scene::LoadComponents(
    const entt::entity entity,
    nlohmann::json &components_data)
{
    component_registry.RegisterSerialization();

    for (auto component_data : components_data)
    {
        const auto type = component_data["type"].get<std::string>();
        const auto component_creation = component_registry.create_method(type);
        component_creation(component_data, m_registry, entity);
    }
}

void Canella::Scene::Load(const std::string &applicationFolder)
{
    const auto project_path = std::filesystem::absolute(applicationFolder);
    for (const auto &file : std::filesystem::directory_iterator(project_path))
    {
        std::string file_name = file.path().filename().string();
        std::string extension = file.path().extension().string();
        if (file_name == "scene.json")
            LoadEntitiesFromDisk(applicationFolder + "\\"+file_name);
    }
}

void Canella::Scene::LoadEntitiesFromDisk(const std::string& filePath)
{

    Serializer::Serialize(this->this->shared_from_this(),filePath)
}
*/
