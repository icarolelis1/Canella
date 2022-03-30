#include "Component/Component.h"


Engine::Component::Component(const std::string _id):id(_id){};

void Engine::Component::onStart(){};
void Engine::Component::onUpdate(float dt){};
bool Engine::Component::isComponentAlive()const {return isActivated;};
void Engine::Component::setActivated(bool b){isActivated=b;};
Engine::COMPONENT_TYPE Engine::Component::getComponentType(){return componentType;};
void Engine::Component::interfaceUI(){};
const std::string Engine::Component::getId(){return id;};

        //    virtual std::shared_ptr<Engine::Compoennt> createComponent() = 0;

        //     virtual void onAwake();
        //     virtual void onStart();
        //     virtual void onUpdate(float dt);
        //     virtual void loadState(nlohmann::json& config);
        //     virtual void saveState(nlohmann::json& config) const ;

        //     void isComponentAlive() const;
        //     void setActivated(bool b);

        // private:
        //     bool isActivated;
        //     const std::string id;
        //     COMPONENT_TYPE componentType;
