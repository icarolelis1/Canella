#pragma once
#ifndef ENTITY
#define ENTITY
#include <json.hpp>
#include <string>
#include "Component/Component.h"
#include <unordered_map>
#include "Transform/Transform.h"
#include <iostream>
#include <mutex>
namespace Canella {

    using ComponentContainerInstance = std::unordered_map<std::string, std::shared_ptr<Component>>; 
    struct ComponentContainer{
        std::shared_ptr<Component> getComponent(std::string id);
        std::shared_ptr<Component> getComponent(Canella::COMPONENT_TYPE type);
        void listComponents() ;
        void addComponent(std::shared_ptr<Canella::Component>,const std::string name);
        bool checkIfExists(const std::string name) const;
        void displayComponentsInpesctor(); //Display on ImGui Interface
        ComponentContainerInstance getAllComponents()const;
        void destroyComponents();
        void setActivated(bool b);
		std::unordered_map<std::string, std::shared_ptr<Component>> components;
        std::mutex componentsMutex;

    };

    class Entity : public std::enable_shared_from_this<Entity>{
        public:
            Entity(const std::string id);
            bool isActivated() const ;
            void setActivated(bool b);
            void loadState(nlohmann::json& config);
            void saveState(nlohmann::json& config) const ;
            void listComponents()  ;
            void displayOnUi();
            void attachComponent(std::shared_ptr<Canella::Component> component);
	        std::shared_ptr<Canella::Component> getComponent(std::string id);
		    std::shared_ptr<Canella::Component> getComponent(Canella::COMPONENT_TYPE componentType);
		    std::shared_ptr<Entity> getSharedPointer();
            void setEnableComponents(bool b);
           std::shared_ptr<Canella::Transform> transform;
            const std::string id;

        private:
            bool activated = true;
            ComponentContainer componentsContainer; 

    };

    using SEntity = std::shared_ptr<Canella::Entity>;
}





#endif