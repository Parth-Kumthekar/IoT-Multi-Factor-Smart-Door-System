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
 * @brief A thread-safe, blocking queue for Event objects.
 * * This class facilitates communication between producer threads (hardware sensors, 
 * network APIs) and consumer threads (the main logic FSM). It uses a monitor 
 * pattern to ensure that concurrent access is safe and efficient.
 */
class EventQueue
{
public:
    /** @brief Default constructor. */
    EventQueue() = default;

    /// Deleted copy constructor to prevent accidental duplication of the synchronization primitives.
    EventQueue(const EventQueue&) = delete;
    /// Deleted assignment operator to prevent accidental duplication of the synchronization primitives.
    EventQueue& operator=(const EventQueue&) = delete;

    /**
     * @brief Destroy the Event Queue object.
     * @details Ensures the queue is shut down so that any blocking threads are released.
     */
    ~EventQueue()
    {
        shutdown();
    }

    /**
     * @brief Adds an event to the queue by copying it.
     * @param event The event to be added.
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
     * @brief Adds an event to the queue by moving it (rvalue).
     * @param event The event to be moved into the queue.
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
     * @brief Blocks the calling thread until an event is available or the queue shuts down.
     * @param[out] outEvent The event popped from the front of the queue.
     * @return true if an event was successfully popped.
     * @return false if the queue was shut down and no events remain.
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
     * @brief Attempts to pop an event without blocking.
     * @param[out] outEvent The event popped if the queue was not empty.
     * @return true if an event was available.
     * @return false if the queue was empty.
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
     * @brief Blocks the calling thread until an event is available, a timeout occurs, or shutdown.
     * @tparam Rep Arithmetic type representing the number of ticks.
     * @tparam Period std::ratio representing the tick period.
     * @param[out] outEvent The event popped if successful.
     * @param timeout The maximum duration to wait.
     * @return true if an event was popped before the timeout.
     * @return false if the timeout was reached or the queue is shut down.
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

    /**
     * @brief Flags the queue as shutting down and wakes all waiting threads.
     */
    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    /** @brief Checks if the shutdown flag has been set. */
    bool isShutdown() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return shutdown_;
    }

    /** @brief Returns true if the queue contains no events. */
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /** @brief Returns the current number of events in the queue. */
    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    /// Mutex for protecting the underlying std::queue and shutdown flag.
    mutable std::mutex mutex_;
    
    /// Condition variable to signal availability of new events or shutdown.
    std::condition_variable cv_;
    
    /// The underlying non-thread-safe container.
    std::queue<Event> queue_;
    
    /// Flag to stop processing and release blocked threads.
    bool shutdown_ = false;
};

#endif
