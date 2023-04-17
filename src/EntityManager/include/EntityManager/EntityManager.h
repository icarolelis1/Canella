#ifndef ENTITY_MAANGER
#define ENTITY_MANAGER
#include "rEntity/Entity.h"
#include <vector>
#include <random>

std::random_device rand_dev;
std::mt19937 generator(rand_dev());
namespace Canella{
    class EntityManager{

    private:
        static std::vector<CanellaRefactor::Entity> entities;

    public :
        static CanellaRefactor::Entity createEntity();
        static void   destroyEntity(const uint32_t id);
    };
};

#endif