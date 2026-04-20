#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Event.h"
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

/**
 * @class EventQueue
 * @brief A thread-safe, blocking queue for synchronizing system events.
 * * This class implements a Producer-Consumer pattern, allowing multiple hardware 
 * threads (NFC, GPIO, Web API) to safely push events to the main logic thread.
 * It utilizes condition variables to eliminate CPU polling, directly addressing 
 * the real-time requirements of the School of Engineering coursework.
 */
class EventQueue
{
public:
    /** @brief Default constructor. */
    EventQueue() = default;

    /** @brief Deleted copy constructor to prevent unsafe thread-sharing. */
    EventQueue(const EventQueue&) = delete;
    /** @brief Deleted assignment operator to enforce strict ownership. */
    EventQueue& operator=(const EventQueue&) = delete;

    /** @brief Destructor ensures a clean shutdown of the synchronization primitives. */
    ~EventQueue()
    {
        shutdown();
    }

    /**
     * @brief Thread-safe push of an event via const reference.
     * @param event The event to be copied into the queue.
     */
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

    /**
     * @brief Thread-safe push of an event via rvalue reference (Move Semantics).
     * @param event The event to be moved into the queue, reducing memory overhead.
     */
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

    /**
     * @brief Blocking wait for an event.
     * * Suspends the calling thread until an event is available or the queue shuts down.
     * This ensures 0% CPU usage while the system is idle.
     * @param outEvent Reference to store the popped event.
     * @return true if an event was successfully retrieved, false if the queue is shutting down.
     */
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

    /**
     * @brief Non-blocking attempt to pop an event.
     * @param outEvent Reference to store the popped event.
     * @return true if an event was available, false otherwise.
     */
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

    /**
     * @brief Timed wait for an event.
     * * Useful for the FSM to check for events while simultaneously handling timeouts.
     * @param outEvent Reference to store the popped event.
     * @param timeout The maximum time to wait before returning false.
     * @return true if an event was retrieved within the timeout period.
     */
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

    /** @brief Signals the queue to stop accepting events and wakes all waiting threads. */
    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    /** @brief Thread-safe check for shutdown status. */
    bool isShutdown() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return shutdown_;
    }

    /** @brief Thread-safe check if the queue is empty. */
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /** @brief Returns the current number of events pending in the queue. */
    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    /** @brief Synchronization primitive for thread safety. */
    mutable std::mutex mutex_;

    /** @brief Condition variable for thread wake-up signaling. */
    std::condition_variable cv_;

    /** @brief Underlying container for the events. */
    std::queue<Event> queue_;

    /** @brief Flag indicating if the system is transitioning to a shutdown state. */
    bool shutdown_ = false;
};

#endif