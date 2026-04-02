#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Event.h"
#include <condition_variable>
#include <mutex>
#include <queue>

class EventQueue
{
public:
    void push(const Event& event)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(event);
        }
        cv_.notify_one();
    }

    bool waitAndPop(Event& outEvent)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty(); });

        outEvent = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<Event> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif