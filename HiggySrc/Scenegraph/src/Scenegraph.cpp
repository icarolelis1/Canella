#include "Scenegraph/Scenegraph.h"

Engine::Node::Node(std::shared_ptr<Engine::Entity>e_ ){ this->entity = std::move(e_);}

Engine::Scenegraph::Scenegraph(const nlohmann::json& config){
	//Initiaize Component Registry
	ComponentRegistry r =  ComponentRegistry::getInstance();

	//Build the Entire SceneGraph
    std::shared_ptr<Engine::Entity> e1 = std::make_shared<Engine::Entity>("Root");
	root = std::make_shared<Node>(e1);

	std::shared_ptr<Engine::Component> transform = Engine::Transform::create(config); 
	
	std::string i = "DSDAS";
	std::cout<<i<<std::endl;

	for (const nlohmann::json& json : config){

		std::shared_ptr<Engine::Entity> entity = std::make_shared<Engine::Entity>(json["Id"]);
		std::shared_ptr<Engine::Node> node = std::make_shared<Engine::Node>(std::move(entity));
		
		if(json["Parent"] == "Root"){
			addNode(node);
		}

		else{
			std::shared_ptr<Engine::Node> father = findById(json["Parent"]);
			if(father->entity){
				addNode(father,node);
			}
		}
		//Build Components
		for(const auto& j: json["Components"]){
			if(r.registry.find(j["type"]) != r.registry.end()){
				//Create and attach the component described in the json file
				entity->attachComponent(r.registry["TRANSFORM"](config));
			}
			else{
				std::cout<<"\t Component Not Registered ~~\n";
			}
			
		}

	}
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

 std::shared_ptr<Engine::Node>  Engine::Scenegraph::findById(const std::string& id){
	return	 findById(root,id);

}
 std::shared_ptr<Engine::Node>  Engine::Scenegraph::findById(std::shared_ptr<Engine::Node> node,const std::string &id){
	 std::list<std::shared_ptr<Engine::Node>>::iterator it = node->childs.begin();
	 if(node->entity->id == id){
		 return node;
	 }
	 else{
		 while(it != node->childs.end()){
			 findById(*it,id);
		 }
	 }

	return std::shared_ptr<Engine::Node>();

 };

void Engine::Scenegraph::updateSceneGraph(std::shared_ptr<Engine::Node> node){

	if(node->parent)
	node->entity->transform->updateModelMatrix(*node->parent->entity->transform.get());

	else {
		node->entity->transform->updateModelMatrix();
	}
	std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();
	while (it != node->childs.end()) {

		updateSceneGraph(*it);
		it++;
	}
}