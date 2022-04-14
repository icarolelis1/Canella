#include <ComponentRegistry/ComponentRegistry.h>

Engine::ComponentRegistry::ComponentRegistry(){
    //Register Transform
    registerComponent("TRANSFORM",Engine::Transform::create);


};

 Engine::ComponentRegistry& Engine::ComponentRegistry::getInstance(){
    static ComponentRegistry componentRegistry;
    return componentRegistry;
};

void Engine::ComponentRegistry::registerComponent(const std::string& type,functionCreation createFunc ){
    registry[type] = createFunc;
}
