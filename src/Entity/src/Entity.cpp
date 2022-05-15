#include <Entity/Entity.h>


//ComponentContainer Definitions
std::shared_ptr<Engine::Component> Engine::ComponentContainer::getComponent(std::string id){
    if(components.find(id)!= components.end())			
        return components[id];
    return nullptr;
};

std::shared_ptr<Engine::Component> Engine::ComponentContainer::getComponent(Engine::COMPONENT_TYPE type){
    std::unordered_map<std::string, std::shared_ptr<Component> >::iterator it;
	it = components.begin();
	while (it != components.end()) {

		if (it->second->getComponentType() == type) {
			return it->second;
			}
			it++;
	
		}

    return nullptr;
}

void Engine::ComponentContainer::setActivated(bool b){
    std::unordered_map<std::string, std::shared_ptr<Component> >::iterator it;
		it = components.begin();
		while (it != components.end()) {	
			it->second->setActivated(b);
			it++;
		}
}
void Engine::ComponentContainer::listComponents(){
 	std::unordered_map<std::string, std::shared_ptr<Component> >::iterator it = components.begin();;
	while (it != components.end()) {
		std::cout << it->first << std::endl;
			it++;
		}

	}

void Engine::ComponentContainer::addComponent(std::shared_ptr<Component>component, std::string name){
	std::lock_guard<std::mutex>lock(componentsMutex);

	if (components.count(name) > 0) {
				name.append(std::to_string(components.count(name) + 1));
			}
			components[name] = std::move(component);
}

//Entity Definitions


void Engine::Entity::listComponents() {
	componentsContainer.listComponents();
}

Engine::Entity::Entity(const std::string _id ):id(_id){
	std::string transformId = _id + "_Transform";
	transform = std::make_shared<Engine::Transform>(transformId);
	componentsContainer.addComponent(transform,transform->getId());
};
void Engine::Entity::attachComponent(std::shared_ptr<Engine::Component> component)
{
	componentsContainer.addComponent(component,component->getId());
}

std::shared_ptr<Engine::Component> Engine::Entity::getComponent(std::string name)
{
	
	return componentsContainer.getComponent(name);

}

std::shared_ptr<Engine::Component> Engine::Entity::getComponent(Engine::COMPONENT_TYPE componentType)
{
	
	if (componentsContainer.getComponent(componentType)!= NULL) {
		return componentsContainer.getComponent(componentType);
	}

	else return nullptr;
}
//Iterate throug all component and display an ImGui Inpsector.
void Engine::Entity::displayOnUi(){

	std::unordered_map<std::string, std::shared_ptr<Engine::Component>>::iterator it = componentsContainer.components.begin();


	while (it != componentsContainer.components.end()) {

	

		// if (ImGui::TreeNode(it->second->getId().c_str())) {
		// 	ImGui::PushID(it->second->getId().c_str());
		// 	it->second->buildUi();
		// 	ImGui::PopID();
		// 	ImGui::TreePop();
		// }
	     it++;

	}
}
void Engine::Entity::setActivated(bool b){
    activated = b;
    if(!activated){
        componentsContainer.setActivated(activated);
    }
}
void Engine::Entity::setEnableComponents(bool b){
    componentsContainer.setActivated(activated);
}