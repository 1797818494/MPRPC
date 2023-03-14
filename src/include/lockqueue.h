#pragma once 
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
// 异步写日志的日志队列
template<typename T>
class LockQueue {
    public:
        void Push(const T&data) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(data);
            m_condvariable.notify_one();
        }
        T Pop() {
            std::unique_lock<std::mutex> lock(m_mutex);
            // 这是为了避免队列为空时，还在不停的抢占锁，不能写入，导致rpc服务变慢
            while(m_queue.empty()) {
                m_condvariable.wait(lock);
            }
            T data = m_queue.front();
            m_queue.pop();
            return data;
        }
    private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};