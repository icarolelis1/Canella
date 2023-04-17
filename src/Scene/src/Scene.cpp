#include "Scene/Scene.h"
#include "Logger/Logger.hpp"

/**
 * \brief Builds the components for each entity in the config File
 * \param Component Registry
 * \param json Data Serialized for each component
 * \param entity entity the component is associated to
 */


/*
Canella::Node::Node(std::shared_ptr<Canella::Entity> e_) { this->entity = std::move(e_); }

Canella::Node::Node() = default;;


void Canella::Scenegraph::buildComponents(
	ComponentRegistry& componentRegistry,
	const nlohmann::json& json,
	std::shared_ptr<Entity> entity)
{
	// for (const auto &j : json["Components"])
	// {
	// 	if (componentRegistry.registry.find(j["type"]) != componentRegistry.registry.end())
	// 		// Create and attach the component described in the json file
	// 		//Canella::ComponentRegistry::getInstance().attachComponent(*entity,  j["type"]);
	// 	else
	// 		Logger::Info("\t Component Not Registered");
	// }
}

void Canella::Scenegraph::build(const nlohmann::json &config)
{
	// Initiaize Component Registry
	ComponentRegistry &r = ComponentRegistry::getInstance();

	// Build the Entire SceneGraph
	auto e1 = std::make_shared<Entity>();
	root = std::make_shared<Node>(e1);

	for (const nlohmann::json &json : config["Entities"])
	{
		auto entity = std::make_shared<Entity>();
		const auto node = std::make_shared<Node>(entity);

		if (json["Parent"] == "Root")
			addNode(node);
		else
		{
			const auto father = findById(json["Parent"]);
			if (father->entity)
			{
				addNode(father, node);
			}
		}
		// Build Components
		buildComponents(r, json, entity);
	}
}
void Canella::Scenegraph::addNode(std::shared_ptr<Node> entity)
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
		if (auto n = std::move(findById(*it, id)); n->entity)
			return n;
		++it;
	};
	return std::make_shared<Canella::Node>();
};

// void Canella::Scenegraph::udpate(std::shared_ptr<Node> node, float dt)
// {
// 	node->entity->update(dt);
// 	auto it = node->childs.begin();
// 	while (it != node->childs.end())
// 	{
// 		udpate(*it, dt);
// 		++it;
// 	};
// }

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
*/