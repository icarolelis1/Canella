#include "Scenegraph/Scenegraph.h"

Engine::Node::Node(std::shared_ptr<Engine::Entity>e_ ){ this->entity = std::move(e_);}
Engine::Scenegraph::Scenegraph(const nlohmann::json& config){
    std::shared_ptr<Engine::Entity> e1 = std::make_shared<Engine::Entity>("Root");
	root = std::make_shared<Node>(e1);
	//root->entity->transform.setScale(glm::vec3(1));
}
void Engine::Scenegraph::addNode(std::shared_ptr<Engine::Node> entity)
{
	root->childs.emplace_back(entity);
	entity->parent = root;
}

void Engine::Scenegraph::addNode(std::shared_ptr<Engine::Node> p1, std::shared_ptr<Node> p2)
{
	p1->childs.emplace_back(p2);
	p2->parent = p1;
}