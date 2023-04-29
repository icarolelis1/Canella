#include "EntityManager/EntityManager.h"
#include <Entity/Entity.h>

std::random_device rand_dev;
std::mt19937 generator(rand_dev());
std::vector<Canella::Entity> Canella::EntityManager::entities ={};

Canella::Entity Canella::EntityManager::createEntity(){
    std::uniform_int_distribution<int>  distr(1, INT_MAX);
    const auto id = distr(generator);
    entities.emplace_back();
    entities.back().id = id;
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