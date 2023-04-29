#pragma once
#ifndef SCENE
#define SCENE
#include "ComponentRegistry/ComponentRegistry.h"
#include "Transform/Transform.h"
#include "Logger/Logger.hpp"
#include "EntityManager/EntityManager.h"
#include <string>
#include <memory>
namespace Canella
{
	struct Node
	{
	public:
		Node(Canella::Entity);
		Node(const Node& other);
		bool operator ==(const Node& other);
		Node() = default;
		Canella::Entity entity;
		Canella::Node *parent = nullptr;
		std::list<Node>childs = std::list<Node>();;
		std::mutex childsMutex;
	};
		
	class Scene
	{
	
	public:
		/**
		 * brief Scene object manages the entities and components in a game scene
		 */
		Scene() = default;
		void build(const nlohmann::json &config);
		void buildComponents(Canella::ComponentRegistry &r, const nlohmann::json &json,
							 Canella::Entity entity);
		EntityManager entities;
		void addNode(Node entity);
		void addNode(Node p1, Node p2);
		void buildUI(Node node);
		void udpate(Canella::Node node, float dt);
		void updateSceneGraph(Canella::Node node);
		void saveState(Node root, std::fstream &saveFile);
		Canella::Node findById(const std::uint32_t &id);
		Node root;
		// std::unique_ptr<Canella::Threadpool> threadpool;
	private:
		std::mutex root_mutex;
		Canella::Node findById(Canella::Node node, const std::uint32_t &id);
		std::mutex entityMutex;
	};

}

#endif