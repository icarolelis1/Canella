#pragma once
#ifndef SCENE
#define SCENE
#include "entt.hpp"
#include <memory>
#include "Logger/Logger.hpp"
#include "Components/Components.h"
#include "json.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace Canella {
    class Entity;
    using EntityLibrary = std::unordered_map<entt::entity, std::shared_ptr<Entity>>;
    class Scene : public std::enable_shared_from_this<Scene> {
    public:

        Scene() = default;
        ~Scene() = default;
        Entity CreateEntity();
        entt::registry m_registry;
        EntityLibrary m_EntityLibrary;
    };
}

#endif