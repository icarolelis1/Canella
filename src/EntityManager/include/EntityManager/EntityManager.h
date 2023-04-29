#pragma once
#ifndef ENTITY_MAANGER
#define ENTITY_MANAGER
#include "Entity/Entity.h"
#include <vector>
#include <random>

namespace Canella{
    class EntityManager{
    private:

    public :
        static std::vector<Entity> entities;
        static Entity createEntity();
        static void   destroyEntity(const uint32_t id);
    };
};

#endif