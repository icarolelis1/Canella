#pragma once
#ifndef COMPONENT_REGISTRY
#define COMPONENT_REGISTRY
#include "ComponentManager/ComponentManager.h"
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include "Entity/Entity.h"

namespace Canella
{
    using functionCreation = std::function<std::shared_ptr<Canella::Component>()>;
    using loadComponent = std::function<std::shared_ptr<Canella::Component>(const nlohmann::json& id)>;
    using RefComponent = std::shared_ptr<Component>;
    using ComponentMap = std::unordered_map<std::string, std::unordered_map<Entity, RefComponent>>;
    
    class ComponentRegistry
    {
    public:
        static ComponentRegistry& getInstance();
        void registerComponent(const std::string& id, functionCreation);
        void initializeRegistry();
        std::map<const std::string, functionCreation> registry;
        RefComponent attachComponent(Entity entity, std::string componentIdentifier);
        void deleteComponent(Entity entity,std::string componentIdentifier);

    private:
        ComponentRegistry(const ComponentRegistry& other) = delete;
        ComponentRegistry& operator=(ComponentRegistry const&) = delete;
        ComponentRegistry();
        ComponentMap components_map;
    };
}

#endif
