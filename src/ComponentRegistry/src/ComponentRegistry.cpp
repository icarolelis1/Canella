#include <ComponentRegistry/ComponentRegistry.h>
#include "Transform/Transform.h"
#include "CameraController/CameraController.h"
Canella::ComponentRegistry::ComponentRegistry(){
    // Register Transform
};

Canella::ComponentRegistry &Canella::ComponentRegistry::getInstance()
{
    static ComponentRegistry componentRegistry;
    return componentRegistry;
};

void Canella::ComponentRegistry::initializeRegistry()
{
    registerComponent("TRANSFORM", Canella::Transform::create);
    registerComponent("CameraController", Canella::CameraController::create);
}

void Canella::ComponentRegistry::registerComponent(const std::string &type, functionCreation createFunc)
{
    registry[type] = createFunc;
}
