#include "EntityManager/EntityManager.h"
#include <Entity/Entity.h>


CanellaRefactor::Entity Canella::EntityManager::createEntity(){
    std::uniform_int_distribution<int>  distr(1, INT_MAX);
    auto id = distr(generator);
    entities.push_back(CanellaRefactor::Entity(id));
    return entities.back();
}

void Canella::EntityManager::destroyEntity(const uint32_t id)
{
    for (int i = 0 ; i < entities.size(); ++i)
        if(entities[i].id == id)
        {
            entities.erase(entities.begin() + i);
            break;
        }
}