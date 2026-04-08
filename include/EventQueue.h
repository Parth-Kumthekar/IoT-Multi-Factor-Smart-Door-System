#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Event.h"
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

class EventQueue
{
public:
    EventQueue() = default;

    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    ~EventQueue()
    {
        shutdown();
    }

    // Push by const reference
    void push(const Event& event)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_)
            {
                return;
            }
            queue_.push(event);
        }
        cv_.notify_one();
    }

    // Push by rvalue reference
    void push(Event&& event)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_)
            {
                return;
            }
            queue_.push(std::move(event));
        }
        cv_.notify_one();
    }

    // Blocking wait. Returns false when queue is shut down and empty.
    bool waitAndPop(Event& outEvent)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() {
            return shutdown_ || !queue_.empty();
        });

        if (queue_.empty())
        {
            return false;
        }

        outEvent = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Non-blocking pop
    bool tryPop(Event& outEvent)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.empty())
        {
            return false;
        }

        outEvent = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Timed wait
    template <typename Rep, typename Period>
    bool waitForAndPop(Event& outEvent, const std::chrono::duration<Rep, Period>& timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        const bool ready = cv_.wait_for(lock, timeout, [this]() {
            return shutdown_ || !queue_.empty();
        });

        if (!ready || queue_.empty())
        {
            return false;
        }

        outEvent = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    bool isShutdown() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return shutdown_;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Event> queue_;
    bool shutdown_ = false;
};

#endif