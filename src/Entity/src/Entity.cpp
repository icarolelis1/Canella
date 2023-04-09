#include <Entity/Entity.h>
uint32_t Canella::Entity::GLOBAL_ENTITY_IDS = 0;

// ComponentContainer Definitions
std::shared_ptr<Canella::Component> Canella::ComponentContainer::getComponent(std::string id)
{
	if (components.find(id) != components.end())
		return components[id];
	return nullptr;
};

std::shared_ptr<Canella::Component> Canella::ComponentContainer::getComponent(Canella::COMPONENT_TYPE type)
{
	auto it = components.begin();
	while (it != components.end())
		if (it->second->getComponentType() == type)
			return it->second;
		++it;
	return nullptr;
}

void Canella::ComponentContainer::update(float dt)
{
	auto it = components.begin();
	while (it != components.end())
	{
		it->second->onUpdate(dt);
		++it;
	}
}

void Canella::ComponentContainer::setActivated(bool b)
{
	auto it = components.begin();
	while (it != components.end())
	{
		it->second->setActivated(b);
		++it;
	}
}
void Canella::ComponentContainer::listComponents()
{
	auto it = components.begin();
	while (it != components.end())
	{
		std::cout << it->first << std::endl;
		++it;
	}
}

void Canella::ComponentContainer::addComponent(std::shared_ptr<Component> component, std::string name)
{
	std::lock_guard<std::mutex> lock(componentsMutex);

	if (components.count(name) > 0)
	{
		name.append(std::to_string(components.count(name) + 1));
	}
	components[name] = std::move(component);
}

void Canella::Entity::listComponents()
{
	componentsContainer.listComponents();
}

Canella::Entity::Entity() : id(GLOBAL_ENTITY_IDS)
{
	std::string transformId = id + "_Transform";
	transform = std::make_shared<Canella::Transform>(transformId);
	componentsContainer.addComponent(transform, transform->getId());
};
void Canella::Entity::attachComponent(std::shared_ptr<Canella::Component> component)
{
	componentsContainer.addComponent(component, component->getId());
}

void Canella::Entity::update(float dt)
{
	componentsContainer.update(dt);
}

std::shared_ptr<Canella::Component> Canella::Entity::getComponent(std::string name)
{

	return componentsContainer.getComponent(name);
}

std::shared_ptr<Canella::Component> Canella::Entity::getComponent(Canella::COMPONENT_TYPE componentType)
{
	if (componentsContainer.getComponent(componentType) != nullptr)
		return componentsContainer.getComponent(componentType);
	return nullptr;
}

void Canella::Entity::displayOnUi()
{
}
void Canella::Entity::setActivated(bool b)
{
	activated = b;
	if (!activated)
	{
		componentsContainer.setActivated(activated);
	}
}
void Canella::Entity::setEnableComponents(bool b)
{
	componentsContainer.setActivated(activated);
}