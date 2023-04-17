#pragma once
#ifndef SCENE
#define SCENE
#include "rEntity/Entity.h"
#include <string>
#include <Transform/Transform.h>
#include "ComponentManager/ComponentManager.h"

namespace Canella
{
	class Scene
	{
	public:
		/**
		 * brief Scene object manages the entities and components in a game scene
		 */
		Scene() = default;
		ComponentLibrary component_library ;
		ComponentManager<TransformData>& transformers = component_library.registerComponent<TransformData>("TransformData");
		
		
	//	void build(const nlohmann::json &config);
	//	void buildComponents(Canella::ComponentRegistry& r, const nlohmann::json& json,
	//		std::shared_ptr<Canella::Entity> entity);
	//	void addNode(std::shared_ptr<Node> entity);
	//	void addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2);
	//	void buildUI(std::shared_ptr<Node> node);
	//	void udpate(std::shared_ptr<Canella::Node> node, float dt);
	//	void updateSceneGraph(std::shared_ptr<Canella::Node> node);
	//	void saveState(std::shared_ptr<Node> root, std::fstream &saveFile);
	//	std::shared_ptr<Canella::Node> findById(const std::uint32_t &id);
	//	std::shared_ptr<Node> root;
	//	// std::unique_ptr<Canella::Threadpool> threadpool;
	//private:
	//	std::mutex root_mutex;
	//	std::shared_ptr<Canella::Node> findById(std::shared_ptr<Canella::Node> node, const std::uint32_t &id);
	//	std::mutex entityMutex;
	};

}

#endif