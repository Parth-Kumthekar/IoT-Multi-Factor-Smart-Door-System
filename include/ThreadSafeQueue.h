#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_.notify_one();  // Wake thread
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); }); // Blocking wait
        T value = queue_.front();
        queue_.pop();
        return value;
    }
};