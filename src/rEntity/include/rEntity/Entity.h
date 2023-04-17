#ifndef CANELLA_ENTITY
#define CANELLA_ENTITY
#include <cstdint>

namespace CanellaRefactor{
    class Entity{
    public:
        Entity(uint32_t id);
        uint32_t id;
    };
}
#endif