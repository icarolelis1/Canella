#pragma once
#ifndef ENTITY
#define ENTITY
#include <json.hpp>
#include <string>
#include <Component/Component.h>
#include <unordered_map>
#include <iostream>
namespace Engine{

    using ComponentContainerInstance = std::unordered_map<std::string, std::shared_ptr<Component>>; 
    struct ComponentContainer{
        std::shared_ptr<Component> getComponent(std::string id);
        std::shared_ptr<Component> getComponent(Engine::COMPONENT_TYPE type);
        void listComponents() ;
        void addComponent(std::shared_ptr<Engine::Component>,const std::string name);
        bool checkIfExists(const std::string name) const;
        void displayComponentsInpesctor(); //Display on ImGui Interface
        ComponentContainerInstance getAllComponents()const;
        void destroyComponents();
        void setActivated(bool b);
		std::unordered_map<std::string, std::shared_ptr<Component>> components;


    };

    class Entity : public std::enable_shared_from_this<Entity>{
        public:
            Entity(const std::string id);
            bool isActivated() const ;
            void setActivated(bool b);
            void loadState(nlohmann::json& config);
            void saveState(nlohmann::json& config) const ;
            void listComponents() const ;
            void displayOnUi();
            void attachComponent(std::shared_ptr<Engine::Component> component);
	        std::shared_ptr<Engine::Component> getComponent(std::string id);
		    std::shared_ptr<Engine::Component> getComponent(Engine::COMPONENT_TYPE componentType);
		    std::shared_ptr<Entity> getSharedPointer();
            void setEnableComponents(bool b);

        private:
            bool activated = true;
            const std::string id;
            ComponentContainer componentsContainer; 

    };
}





#endif