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

namespace Canella {
	struct Node {
	public:
		Node(std::shared_ptr<Canella::Entity>);
		Node();
		std::shared_ptr<Canella::Entity> entity;
		std::shared_ptr<Canella::Node> parent;
		std::list<std::shared_ptr<Node>>childs;
		std::mutex childsMutex;
	};

	class Scenegraph {
	public:
		Scenegraph(const nlohmann::json& config);
		void addNode(std::shared_ptr<Node> entity);
		void addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2);
		void buildUI(std::shared_ptr<Node> node);
		void updateSceneGraph(std::shared_ptr<Canella::Node> node);
		void saveState(std::shared_ptr<Node> root, std::fstream& saveFile);
		std::shared_ptr<Canella::Node>  findById(const std::string& id);
		
		//std::unique_ptr<Canella::Threadpool> threadpool;
	private:
		std::mutex root_mutex;
		std::shared_ptr<Node> root;
		std::shared_ptr<Canella::Node> findById(std::shared_ptr<Canella::Node> node, const std::string& id);
		std::mutex entityMutex;
	};


}

#endif