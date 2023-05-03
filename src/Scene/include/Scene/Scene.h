#pragma once
#ifndef SCENE
#define SCENE
#include "entt.hpp"
#include "Logger/Logger.hpp"
#include "Components/Components.h"
#include "json.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
namespace Canella
{
	class Entity;
	using EntityLibrary = std::unordered_map<entt::entity, Entity >;
	class Scene : public std::enable_shared_from_this< Scene >
	{
	public:
		Scene() = default;
		Entity CreateEntity();
		entt::registry m_registry;
		EntityLibrary m_EntityLibrary;
	private:
		void LoadComponents(const entt::entity,nlohmann::json&);
		void LoadEntitiesFromDisk(const std::string& path);
		std::shared_ptr<Scene> sharedPtr;
	};
}

#endif