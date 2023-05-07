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
        Entity(entt::entity entity,std::weak_ptr<Scene> scene) : m_Scene(scene),handle(entity){}

        template<typename T,typename... Args>
        T& AddComponent(Args&&... args)
        {
            assert(!HasComponent<T>());
            assert(!m_Scene.expired());
            auto scene =  m_Scene.lock();
            return scene->m_registry.emplace<T>(handle,std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            assert(HasComponent<T>());
            assert(!m_Scene.expired());
            auto scene = m_Scene.lock();
            return scene->m_registry.get<T>(handle);
        }

        template<typename  T>
        void RemoveComponent()
        {
            assert(HasComponent<T>());
            auto scene = m_Scene.lock();
            assert(m_Scene.expired());
            scene->m_registry.remove<T>(handle);
        }

        template<typename T>
        bool HasComponent()
        {
            auto scene =  m_Scene.lock();
            if(scene)
                return scene->m_registry.any_of<T>(handle);
            return false;
        }

        entt::entity RawId()
        {
            return handle;
        }
        
    private:
        std::weak_ptr<Scene> m_Scene;
        entt::entity handle;
    };
}

#endif