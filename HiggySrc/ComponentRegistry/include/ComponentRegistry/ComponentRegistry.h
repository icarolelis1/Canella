#pragma once
#ifndef COMPONENT_REGISTRY
#define COMPONENT_REGISTRY
#include "Transform/Transform.h"
#include <map>
#include <memory>
#include <string>
namespace Engine{
        using functionCreation = std::function<std::shared_ptr<Engine::Component>(const nlohmann::json& id)>;
        class ComponentRegistry{
            public:
                static ComponentRegistry& getInstance();
                void registerComponent(const std::string& id,functionCreation);
                void initializeRegistry();
                std::map<const std::string,functionCreation> registry;

            private:
                ComponentRegistry(const ComponentRegistry& other) = delete;
                ComponentRegistry& operator=(ComponentRegistry const&) = delete;
                ComponentRegistry();

        };
    
}

#endif