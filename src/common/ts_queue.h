#pragma once
#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

//add thread safety to deque
//basically add scope locks in all the operations
//and the ones that add things in the container (push_back, emplace_back, ...)
//we use a contion variable to notify the thread waiting (wait function) for this resource
//to check the condition and unlock the mutex if condition is met
template<typename T>
class ts_queue 
{
public:
    ts_queue() = default;
    ts_queue(const ts_queue&) = delete;
    virtual ~ts_queue() { clear(); }

    const T& front()
    {
        std::scoped_lock lock(m_MutexQueue);
        return m_Queue.front();
    }

    const T& back()
    {
        std::scoped_lock lock(m_MutexQueue);
        return m_Queue.back();
    }

    T pop_front()
    {
        std::scoped_lock lock(m_MutexQueue);
        T temp = std::move(m_Queue.front());
        m_Queue.pop_front();
        return temp;
    }

    T pop_back()
    {
        std::scoped_lock lock(m_MutexQueue);
        T temp = std::move(m_Queue.back());
        m_Queue.pop_back();
        return temp;
    }

    void push_front(T&& val)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.push_front(std::move(val));

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    void push_back(T&& val)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.push_back(std::move(val));

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    void push_front(const T& val)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.push_front(val);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    void push_back(const T& val)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.push_back(val);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    template<typename... Args>
    void emplace_front(Args&&... args)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.emplace_front(std::forward<Args>(args)...);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        std::scoped_lock sLock(m_MutexQueue);
        m_Queue.emplace_back(std::forward<Args>(args)...);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    bool empty()
    {
        std::scoped_lock lock(m_MutexQueue);
        return m_Queue.empty();
    }

    size_t size()
    {
        std::scoped_lock lock(m_MutexQueue);
        return m_Queue.size();
    }

    void clear()
    {
        std::scoped_lock lock(m_MutexQueue);
        m_Queue.clear();
    }

    void wait()
    {
        std::unique_lock lock(m_MutexCV);
        m_CV.wait(lock, [this]() { return !empty(); });
    }

private:
    std::deque<T> m_Queue;
    std::condition_variable m_CV;
    std::mutex m_MutexQueue;
    std::mutex m_MutexCV;
};
