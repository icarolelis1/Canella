#include "Component/Component.h"


Engine::Component::Component(const std::string _id):id(_id){};

void Engine::Component::onStart(){};
void Engine::Component::onUpdate(float dt){};
bool Engine::Component::isComponentAlive()const {return isActivated;};
void Engine::Component::setActivated(bool b){isActivated=b;};
Engine::COMPONENT_TYPE Engine::Component::getComponentType(){return componentType;};
void Engine::Component::interfaceUI(){};
const std::string Engine::Component::getId(){return id;};

