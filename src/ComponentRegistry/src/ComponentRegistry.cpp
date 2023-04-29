#include <ComponentRegistry/ComponentRegistry.h>

/**
 * \brief Registers create method for all components
 */
Canella::ComponentRegistry::ComponentRegistry() = default;;

Canella::ComponentRegistry &Canella::ComponentRegistry::getInstance()
{
    static ComponentRegistry component_registry;
    return component_registry;
};

void Canella::ComponentRegistry::initializeRegistry()
{
    registerComponent("TRANSFORM", Canella::Transform::create);
    registerComponent("CameraController", Canella::CameraController::create);
}

void Canella::ComponentRegistry::registerComponent(const std::string &id, functionCreation create_func)
{
    registry[id] = std::move(create_func);
}
/**
 * \brief Creates a new Component and attach to Entity
 * \param entity entity associated with the component
 * \param component_identifier String identifier of the Component Type
 * \return a shared_pointer to the created component
 */

Canella::RefComponent Canella::ComponentRegistry::attachComponent(Entity entity, std::string component_identifier)
{
    assert(components_map[component_identifier].find(entity.id) == components_map[component_identifier].end());
    components_map[component_identifier][entity.id] = registry[component_identifier]();
    return components_map[component_identifier][entity.id];
}

void Canella::ComponentRegistry::deleteComponent(Entity entity, std::string componentIdentifier)
{
    assert(components_map[componentIdentifier].find(entity.id) == components_map[componentIdentifier].end());
    components_map[componentIdentifier].erase(entity.id);
}
