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
        Entity(entt::entity entity,std::weak_ptr<Scene> scene) : m_Scene(scene),handle(entity){}

        template<typename T,typename... Args>
        T& add_component(Args&&... args)
        {
            assert(!has_component<T>());
            assert(!m_Scene.expired());
            auto scene =  m_Scene.lock();
            return scene->m_registry.emplace<T>(handle,std::forward<Args>(args)...);
        }

        template<typename T>
        T& get_component()
        {
            assert(has_component<T>());
            assert(!m_Scene.expired());
            auto scene = m_Scene.lock();
            return scene->m_registry.get<T>(handle);
        }

        template<typename  T>
        void remove_component()
        {
            assert(has_component<T>());
            auto scene = m_Scene.lock();
            assert(m_Scene.expired());
            scene->m_registry.remove<T>(handle);
        }

        template<typename T>
        bool has_component()
        {
            auto scene =  m_Scene.lock();
            if(scene)
                return scene->m_registry.any_of<T>(handle);
            return false;
        }

        entt::entity raw_id()
        {
            return handle;
        }

    private:
        std::weak_ptr<Scene> m_Scene;
        entt::entity handle;
    };
}

#endif