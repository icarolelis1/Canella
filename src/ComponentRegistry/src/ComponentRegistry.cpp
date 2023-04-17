#include <ComponentRegistry/ComponentRegistry.h>
#include "Transform/Transform.h"
#include "CameraController/CameraController.h"
#include <utility>
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
 * \param componentIdentifier String identifier of the Component Type
 * \return a shared_pointer to the created component
 */

/*Canella::RefComponent Canella::ComponentRegistry::attachComponent(Entity entity, std::string componentIdentifier)
{
    assert(components_map[componentIdentifier].find(entity) == components_map[componentIdentifier].end());
    components_map[componentIdentifier][entity] = registry[componentIdentifier]();
    return components_map[componentIdentifier][entity];
}

void Canella::ComponentRegistry::deleteComponent(Entity entity, std::string componentIdentifier)
{
    assert(components_map[componentIdentifier].find(entity) == components_map[componentIdentifier].end());
    components_map[componentIdentifier].erase(entity);
}*/
