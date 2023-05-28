#include "Entity.h"
#include "Scene/Scene.h"


/**
 * \brief brief Create a new Entity in this scene
 * \return The Entity created
 */
Canella::Entity Canella::Scene::CreateEntity()
{
    entt::entity entt_entity = m_registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr );
    m_EntityLibrary[entt_entity] = entity;
    return *entity;
}

/**
 * \brief Builds the components for each entity in the config File
 * \param entity entity the component is associated to
 * \param components_data Data Serialized for each component
 */
