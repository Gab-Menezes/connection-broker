#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>

//add thread safety to vector
//basically add scope locks in all the operations
//and the ones that add things in the container (push_back, emplace_back, ...)
//we use a contion variable to notify the thread waiting (wait function) for this resource
//to check the condition and unlock the mutex if condition is met
template<typename T>
class ts_vector 
{
public:
    ts_vector() = default;
    ts_vector(const ts_vector&) = delete;
    virtual ~ts_vector() { clear(); }

    void push_back(T&& val)
    {
        std::scoped_lock sLock(m_MutexVector);
        m_Vector.push_back(std::move(val));

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    void push_back(const T& val)
    {
        std::scoped_lock sLock(m_MutexVector);
        m_Vector.push_back(val);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        std::scoped_lock sLock(m_MutexVector);
        m_Vector.emplace_back(std::forward<Args>(args)...);

        std::unique_lock uLock(m_MutexCV);
        m_CV.notify_one();
    }

    const T& back()
    {
        std::scoped_lock sLock(m_MutexVector);
        return m_Vector.back();
    }

    const T& front()
    {
        std::scoped_lock sLock(m_MutexVector);
        return m_Vector.front();
    }

    bool empty()
    {
        std::scoped_lock lock(m_MutexVector);
        return m_Vector.empty();
    }

    size_t size()
    {
        std::scoped_lock lock(m_MutexVector);
        return m_Vector.size();
    }

    void clear()
    {
        std::scoped_lock lock(m_MutexVector);
        m_Vector.clear();
    }

    void wait()
    {
        std::unique_lock lock(m_MutexCV);
        m_CV.wait(lock, [this]() { return !empty(); });
    }

    //utility function for cleaning the vector
    void remove_if(const std::function<bool(const T&)>& func)
    {
        std::scoped_lock lock(m_MutexVector);
        m_Vector.erase(std::remove_if(m_Vector.begin(), m_Vector.end(),
            func), m_Vector.end());
    }

private:
    std::vector<T> m_Vector;
    std::condition_variable m_CV;
    std::mutex m_MutexVector;
    std::mutex m_MutexCV;
};