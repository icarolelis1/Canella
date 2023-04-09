#include <ComponentRegistry/ComponentRegistry.h>

#include <utility>
#include "Transform/Transform.h"
#include "CameraController/CameraController.h"
/**
 * \brief 
 */
Canella::ComponentRegistry::ComponentRegistry(){};

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
