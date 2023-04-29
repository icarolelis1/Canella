#pragma once
#ifndef ENTITY
#define ENTITY
#include "EntityBase.h"
#include <json.hpp>
#include <string>
#include <mutex>

namespace Canella
{

    class Entity : public EntityBase
    {
    public:
        Entity() = default;
        Entity(const Entity& other);

    private:
        bool activated = true;
    };
}

#endif