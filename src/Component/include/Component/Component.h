#pragma once
#ifndef COMPONENT
#define COMPONENT
#include <string>
#include <memory>
#include <json.hpp>\

namespace Canella {

enum class COMPONENT_TYPE
	{
		SCRIPT,
		COLLISOR,
		MESH,
        TRANSFORM
	};

    class Component{
        public:
            Component(const std::string ComponentID);   
            const std::string getId();
            virtual void onAwake();
            virtual void onStart();
            virtual void onUpdate(float dt);
            virtual void loadState(nlohmann::json& config)=0;
            virtual void saveState(nlohmann::json& config) =0 ;
            virtual void interfaceUI();
            COMPONENT_TYPE getComponentType();
            void setComponentType(Canella::COMPONENT_TYPE type);
            bool isComponentAlive() const;
            void setActivated(bool b);
            void setId(std::string id);

        private:
            bool isActivated;
            const std::string id;
            COMPONENT_TYPE componentType;



    };

}

#endif