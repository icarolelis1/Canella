#include "Scene/Scene.h"

/**
 * \brief Builds the components for each entity in the config File
 * \param Component Registry
 * \param json Data Serialized for each component
 * \param entity entity the component is associated to
 */

Canella::Node::Node(Entity e_): parent(nullptr)
{
	this->entity = e_;
}

Canella::Node::Node(const Node& e)
{
	this->entity = e.entity;
};

bool Canella::Node::operator==(const Node& n)
{
	return entity.id == n.entity.id;
}

void Canella::Scene::buildComponents(
	ComponentRegistry &componentRegistry,
	const nlohmann::json &json,
	Entity entity)
{
	 for (const auto &j : json["Components"])
	 {
	 	if (componentRegistry.registry.find(j["type"]) != componentRegistry.registry.end())
	 		// Create and attach the component described in the json file
	 		ComponentRegistry::getInstance().attachComponent(entity,  j["type"]);
	 	else
	 		Logger::Info("\t Component Not Registered");
	 }
}

void Canella::Scene::build(const nlohmann::json &config)
{
	// Initiaize Component Registry
	ComponentRegistry &r = ComponentRegistry::getInstance();

	// Build the Entire SceneGraph
	const auto e1 = Entity();
	root.entity = e1;

	for (const nlohmann::json &json : config["Entities"])
	{
		auto entity = entities.createEntity();
		Node node;
		node.entity = entity;

		if (json["Parent"] == "Root")
			addNode(node);
		else
		{
			//const auto father = findById(json["Parent"]);
			// if (father->entity)
			// {
			// 	addNode(father, node);
			// }
		}
		// Build Components
		buildComponents(r, json, entity);
	}
}
void Canella::Scene::addNode(Node entity)
{
	std::lock_guard<std::mutex> lock(entity.childsMutex);
	root.childs.push_back(entity);
	entity.parent = &root;
}

void Canella::Scene::addNode(Node p1, Node p2)
{
	std::lock_guard lock(p1.childsMutex);
	p1.childs.push_back(p2);
	//p2.parent.entity = p1.entity;
}

/*
Canella::Node Canella::Scene::findById(const std::uint32_t &id)
{
	return findById(root, id);
}
Canella::Node Canella::Scene::findById(Canella::Node node, const std::uint32_t &id)
{
	std::lock_guard lock(node.childsMutex);
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
*/


