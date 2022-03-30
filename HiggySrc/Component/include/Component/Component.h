#pragma once
#ifndef COMPONENT
#define COMPONENT
#include <string>
#include <memory>
#include <json.hpp>
namespace Engine{

enum class COMPONENT_TYPE
	{
		SCRIPT,
		COLLISOR,
		MESH
	};

    class Component{
        public:
            Component(const std::string ComponentID);   
           virtual std::shared_ptr<Engine::Component> createComponent() = 0;
            const std::string getId();
            virtual void onAwake();
            virtual void onStart();
            virtual void onUpdate(float dt);
            virtual void loadState(nlohmann::json& config)=0;
            virtual void saveState(nlohmann::json& config) const=0 ;
            virtual void interfaceUI();
            COMPONENT_TYPE getComponentType();
            bool isComponentAlive() const;
            void setActivated(bool b);

        private:
            bool isActivated;
            const std::string id;
            COMPONENT_TYPE componentType;

    };

}

#endif