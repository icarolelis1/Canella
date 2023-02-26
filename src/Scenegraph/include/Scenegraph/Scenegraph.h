#pragma once
#ifndef SCENE_GRAPH
#define SCENE_GRAPH
#include "Entity/Entity.h"
#include <list>
#include <memory>
#include <json.hpp>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "ComponentRegistry/ComponentRegistry.h"

namespace Canella
{
	struct Node
	{
	public:
		Node(std::shared_ptr<Canella::Entity>);
		Node();
		std::shared_ptr<Canella::Entity> entity;
		std::shared_ptr<Canella::Node> parent;
		std::list<std::shared_ptr<Node>> childs;
		std::mutex childsMutex;
	};

	class Scenegraph
	{
	public:
		Scenegraph() = default;
		void build(const nlohmann::json &config);
		void addNode(std::shared_ptr<Node> entity);
		void addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2);
		void buildUI(std::shared_ptr<Node> node);
		void udpate(std::shared_ptr<Canella::Node> node, float dt);
		void updateSceneGraph(std::shared_ptr<Canella::Node> node);
		void saveState(std::shared_ptr<Node> root, std::fstream &saveFile);
		std::shared_ptr<Canella::Node> findById(const std::uint32_t &id);
		std::shared_ptr<Node> root;
		// std::unique_ptr<Canella::Threadpool> threadpool;
	private:
		std::mutex root_mutex;
		std::shared_ptr<Canella::Node> findById(std::shared_ptr<Canella::Node> node, const std::uint32_t &id);
		std::mutex entityMutex;
	};

}

#endif