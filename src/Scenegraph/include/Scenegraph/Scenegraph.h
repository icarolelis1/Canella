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
#include "Threadpool/Threadpool.h"
namespace Engine{
struct Node{
    public:
        Node(std::shared_ptr<Engine::Entity>);  
        Node();
        std::shared_ptr<Engine::Entity> entity;
         std::shared_ptr<Engine::Node> parent;
        std::list<std::shared_ptr<Node>>childs;
        std::mutex childsMutex;
};

class Scenegraph{
    public:
        Scenegraph(const nlohmann::json& config);
        void addNode(std::shared_ptr<Node> entity);
        void addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2);
        void buildUI(std::shared_ptr<Node> node );
        void updateSceneGraph(std::shared_ptr<Engine::Node> node);
        void saveState(std::shared_ptr<Node> root, std::fstream& saveFile);
        std::shared_ptr<Engine::Node>  findById(const std::string & id);
        std::unique_ptr<Engine::Threadpool> threadpool;
	private:
        std::mutex root_mutex;
        std::shared_ptr<Node> root;
        std::shared_ptr<Engine::Node> findById(std::shared_ptr<Engine::Node> node,const std::string& id);
        std::mutex entityMutex;
};


}

#endif