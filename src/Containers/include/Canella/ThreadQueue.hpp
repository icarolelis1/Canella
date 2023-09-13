#pragma once
#ifndef THREAD_QUEUE
#define THREAD_QUEUE
#include <thread>
#include <functional>
#include <queue>
#include <memory>
#include <mutex>
#include <vector>
#include <iostream>
#include <Logger/Logger.hpp>

namespace Canella {

    class join_threads
    {
        std::vector<std::thread>& threads;
    public:
        explicit join_threads(std::vector<std::thread>& threads_) :
            threads(threads_)
        {}
        ~join_threads()
        {
            for (unsigned long i = 0; i < threads.size(); ++i)
            {
                if (threads[i].joinable())
                    threads[i].join();
            }
        }
    };

    template<typename T>
    class ThreadQueue {
        mutable std::mutex m;
        std::condition_variable cv;
        std::queue<T> queue;
    public:
        ThreadQueue() {}

        ThreadQueue(ThreadQueue const& other) = delete;
        void push(T item) {
            std::lock_guard<std::mutex> lg(m);
            queue.push(item);
            cv.notify_one();
        }
        bool pop(T& item) {
            return true;
        }
        std::shared_ptr<T> pop() {
            std::lock_guard<std::mutex> lg(m);
            if (queue.empty()) {
                return std::shared_ptr<T>();
            }
            else {
                std::shared_ptr<T>  ref(queue.front());
                queue.pop();
                return ref;
            }
        }
        void waitAndPop(T& ref);

        std::shared_ptr<T> wait_pop() {
            std::unique_lock<std::mutex>lg(m);
            cv.wait(lg, [this] {
                return !queue.empty();
                });
            std::shared_ptr<T> ref = queue.front();
            queue.pop();
            return ref;
        }

        bool empty() {
            std::lock_guard<std::mutex>lg(m);
            return queue.empty();
        }

        bool try_pop(T& item) {
            std::lock_guard<std::mutex>lg(m);
            if (queue.empty()) {
                return false;
            };
            item = queue.front();
            queue.pop();
            return true;
        }

        std::shared_ptr<T> try_pop() {
            std::lock_guard<std::mutex>lg(m);
            if (queue.empty()) {
                return std::shared_ptr<T>();
            }
            std::shared_ptr<T> res(std::make_shared<T>(queue.front()));
            queue.pop();
            return res;
        }
    };
}

#endif