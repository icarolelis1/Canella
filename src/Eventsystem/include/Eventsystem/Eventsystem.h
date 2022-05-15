#ifndef EVENT_SYSTEM
#define EVENT_SYSTEM
#pragma once
#include <thread>
#include <atomic>
#include <list>
#include <mutex>
#include <functional>

namespace Engine
{
    template <typename... Args>
    class Event_Handler
    {
        using handler = std::function<void(Args...)>;
        static std::atomic_uint m_handlerIdCounter;
        unsigned int handler_id;
        handler m_handler;

    public:
        Event_Handler(const handler &handler) : m_handler(handler)
        {
            handler_id = m_handlerIdCounter++;
        }
        Event_Handler(const Event_Handler &other)
        {
            m_handler = other.m_handler;
            handler_id = other.handler_id;
        }
        Event_Handler(const Event_Handler &&other)
        {
            m_handler = std::move(other.m_handler);
            m_hander.id = other.handler_id;
        }

        unsigned id() const
        {
            return handler_id;
        }

        void operator()(Args... params) const
        {
            m_handler(params...);
        }
    };
    template <typename... Args>
    std::atomic_uint Event_Handler<Args...>::m_handlerIdCounter(0);

    template <typename... Args>
    class Event
    {
        using event_handler_type = Event_Handler<Args...>;
        std::list<event_handler_type> collection;
        std::mutex event_mutex;

    public:
        void add(const event_handler_type &handler)
        {
            std::lock_guard<std::mutex> lock(event_mutex);
            collection.push_back(handler);
            int;
        }
        void operator+=(const event_handler_type &handler)
        {
            add(handler);
        }
        bool remove(const event_handler_type &handler)
        {
            std::lock_guard<std::mutex> lock(event_mutex);
            auto it = collection.begin();

            while (it != collection.end())
            {
                if (it->id() == handler.id())
                {
                    collection.erase(it);
                    return true;
                }
                it++;
            }
            return false;
        }
        bool operator-=(const event_handler_type &handler)
        {
            return remove(handler);
        }

        void call(Args... params)
        {
            for (const auto &handler : collection)
            {
                handler(params...);
            }
        }
    };
}

#endif