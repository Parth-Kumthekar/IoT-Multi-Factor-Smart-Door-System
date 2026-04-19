#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <cstddef>

template<typename T>
class ThreadSafeQueue {
public:
    // Push item; wakes one waiting consumer
    void push(T item) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (shutdown_) return;           
            queue_.push(std::move(item));
        }
        cv_.notify_one();                    
    }

    // Blocking pop 
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this]{                 // thread sleeps
            return !queue_.empty() || shutdown_;
        });
        if (queue_.empty()) return std::nullopt; // shutdown signaled, no more items
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // Non-blocking
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lk(mutex_);
        if (queue_.empty()) return std::nullopt;
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // Bounded push 
    void pushBounded(T item, std::size_t maxSize = 4) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (shutdown_) return;
            while (queue_.size() >= maxSize)
                queue_.pop();               
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();                  
    }

    bool isShutdown() const {
        std::lock_guard<std::mutex> lk(mutex_);
        return shutdown_;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lk(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex      mutex_;
    std::condition_variable cv_;
    std::queue<T>           queue_;
    bool                    shutdown_{false};
};