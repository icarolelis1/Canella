#pragma once
#ifndef COMPONENT_REGISTRY
#define COMPONENT_REGISTRY
#include "CameraController/CameraController.h"
#include "Transform/Transform.h"
#include "Entity/Entity.h"
#include <map>
#include <unordered_map>
#include <memory>
#include <string>

namespace Canella
{
    using functionCreation = std::function<std::shared_ptr<Canella::Component>()>;
    using loadComponent = std::function<std::shared_ptr<Canella::Component>(const nlohmann::json& id)>;
    using RefComponent = std::shared_ptr<ComponentBase>;
    using ComponentMap = std::unordered_map<std::string, std::unordered_map<uint32_t, std::shared_ptr<ComponentBase>>>;
    
    class ComponentRegistry
    {
    public:
        static ComponentRegistry& getInstance();
        void registerComponent(const std::string& id, functionCreation);
        void initializeRegistry();
        std::map<const std::string, functionCreation> registry;
        RefComponent attachComponent(Entity entity, std::string componentIdentifier);
        void deleteComponent(Entity entity,std::string componentIdentifier);
        ComponentMap components_map;

    private:
        ComponentRegistry(const ComponentRegistry& other) = delete;
        ComponentRegistry& operator=(ComponentRegistry const&) = delete;
        ComponentRegistry();
    };
}

#endif
