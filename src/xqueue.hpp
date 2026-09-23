
#pragma once

#include <initializer_list>
#include <mutex>
#include <queue>

namespace xqueue
{

template <typename T> struct Queue
{
        std::mutex    mutex;
        std::queue<T> queue;

        Queue() : mutex(std::mutex()), queue(std::queue<T>()) {}

        bool empty() { return this->queue.empty(); }

        void push(T&& t)
        {
                this->mutex.lock();

                this->queue.push(std::move(t));

                this->mutex.unlock();
        }

        void push_range(std::initializer_list<T> list)
        {
                this->mutex.lock();

                this->queue.push_range(std::move(list));

                this->mutex.unlock();
        }

        T pop()
        {
                this->mutex.lock();

                T t = std::move(this->queue.front());
                this->queue.pop();

                this->mutex.unlock();

                return t;
        }
};

} // namespace xqueue
