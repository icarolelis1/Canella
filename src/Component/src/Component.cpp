#include "Component/Component.h"

/**
 * \brief 
 * \param _id
 */
Canella::Component::Component(const std::string _id): isActivated(true), id(_id), componentType()
{};

void Canella::Component::onStart(){};
void Canella::Component::onUpdate(float dt){};
bool Canella::Component::isComponentAlive() const {return isActivated;};
void Canella::Component::setActivated(bool b){isActivated=b;};
Canella::COMPONENT_TYPE Canella::Component::getComponentType(){return componentType;};
void Canella::Component::interfaceUI(){};
const std::string Canella::Component::getId(){return id;};
void Canella::Component::setComponentType(Canella::COMPONENT_TYPE _type){componentType =_type; };
void Canella::Component::onAwake(){};
