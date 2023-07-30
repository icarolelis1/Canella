#pragma once
#ifndef CANELLA_ENTITY
#define CANELLA_ENTITY
#include "entt.hpp"
#include "Scene/Scene.h"
#include <memory>
#include <iomanip>

namespace Canella
{
    class Entity
    {
    public:
        Entity() = default;
        ~Entity() = default;
        Entity(entt::entity entity,std::weak_ptr<Scene> scene) : owner_scene( scene), handle( entity){}

        template<typename T,typename... Args>
        T& add_component(Args&&... args)
        {
            assert(!has_component<T>());
            assert(!owner_scene.expired());
            auto scene =  owner_scene.lock();
            return scene->registry.emplace<T>( handle, std::forward<Args>( args)...);
        }

        template<typename T>
        T& get_component()
        {
            assert(has_component<T>());
            assert(!owner_scene.expired());
            auto scene = owner_scene.lock();
            return scene->registry.get<T>( handle);
        }

        template<typename  T>
        void remove_component()
        {
            assert(has_component<T>());
            auto scene = owner_scene.lock();
            assert( owner_scene.expired());
            scene->registry.remove<T>( handle);
        }

        template<typename T>
        bool has_component()
        {
            auto scene =  owner_scene.lock();
            if(scene)
                return scene->registry.any_of<T>( handle);
            return false;
        }

        entt::entity raw_id()
        {
            return handle;
        }

        std::weak_ptr<Scene> get_owner_scene()
        {
            return owner_scene;
        }

    private:
        std::weak_ptr<Scene> owner_scene;
        entt::entity         handle;
    };
}

#endif