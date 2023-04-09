#include "Scenegraph/Scenegraph.h"
#include "Logger/Logger.hpp"

Canella::Node::Node(std::shared_ptr<Canella::Entity> e_) { this->entity = std::move(e_); }

Canella::Node::Node(){};

void Canella::Scenegraph::build(const nlohmann::json &config)
{
	// Initiaize Component Registry
	ComponentRegistry &r = ComponentRegistry::getInstance();
	r.initializeRegistry();

	// Build the Entire SceneGraph
	std::shared_ptr<Canella::Entity> e1 = std::make_shared<Canella::Entity>();
	root = std::make_shared<Node>(e1);

	for (const nlohmann::json &json : config["Entities"])
	{
		std::shared_ptr<Canella::Entity> entity = std::make_shared<Canella::Entity>();
		std::shared_ptr<Canella::Node> node = std::make_shared<Canella::Node>(entity);

		if (json["Parent"] == "Root")
		{
			addNode(node);
		}
		else
		{
			std::shared_ptr<Canella::Node> father = findById(json["Parent"]);
			if (father->entity)
			{
				addNode(father, node);
			}
		}
		// Build Components
		for (const auto &j : json["Components"])
		{
			if (r.registry.find(j["type"]) != r.registry.end())
			{
				// Create and attach the component described in the json file
				std::string c = j["type"];
				entity->attachComponent(r.registry[j["type"]](j));
			}
			else
			{
				Canella::Logger::Info("\t Component Not Registered");
			}
		};
	}
}
void Canella::Scenegraph::addNode(std::shared_ptr<Canella::Node> entity)
{
	std::lock_guard<std::mutex> lock(entity->childsMutex);
	root->childs.emplace_back(entity);
	entity->parent = root;
}

void Canella::Scenegraph::addNode(std::shared_ptr<Canella::Node> p1, std::shared_ptr<Node> p2)
{
	std::lock_guard lock(p1->childsMutex);
	p1->childs.emplace_back(p2);
	p2->parent = p1;
}

std::shared_ptr<Canella::Node> Canella::Scenegraph::findById(const std::uint32_t &id)
{
	return findById(root, id);
}
std::shared_ptr<Canella::Node> Canella::Scenegraph::findById(std::shared_ptr<Canella::Node> node, const std::uint32_t &id)
{
	std::lock_guard lock(node->childsMutex);
	if (node->entity->id == id)
		return node;
	
	auto it = node->childs.begin();
	while (it != node->childs.end())
	{
		std::shared_ptr<Canella::Node> n;
		n = std::move(findById(*it, id));
		if (n->entity)
			return n;
		++it;
	};
	return std::make_shared<Canella::Node>();
};

void Canella::Scenegraph::udpate(std::shared_ptr<Canella::Node> node, float dt)
{
	node->entity->update(dt);
	auto it = node->childs.begin();
	while (it != node->childs.end())
	{
		udpate(*it, dt);
		++it;
	};
}

void Canella::Scenegraph::updateSceneGraph(const std::shared_ptr<Canella::Node> node)
{
	if (node->parent)
		node->entity->transform->updateModelMatrix(*node->parent->entity->transform);
	else
		node->entity->transform->updateModelMatrix();
	auto it = node->childs.begin();
	while (it != node->childs.end())
	{
		updateSceneGraph(*it);
		++it;
	}
}