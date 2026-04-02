#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include "AsyncLogger.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

class AlarmManager
{
public:
    AlarmManager() = default;

    ~AlarmManager()
    {
        stop();
    }

    void start(AsyncLogger& logger)
    {
        if (running_)
        {
            return;
        }

        logger_ = &logger;
        running_ = true;
        thread_ = std::thread(&AlarmManager::run, this);
    }

    void stop()
    {
        if (!running_)
        {
            return;
        }

        running_ = false;
        cv_.notify_all();

        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    void triggerAlarm(const std::string& reason)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            alarmActive_ = true;
            lastReason_ = reason;
        }
        cv_.notify_one();
    }

    void clearAlarm()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            alarmActive_ = false;
        }
        cv_.notify_one();
    }

    bool isAlarmActive() const
    {
        return alarmActive_.load();
    }

private:
    void run()
    {
        while (running_)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return !running_ || stateChangedUnsafe(); });

            if (!running_)
            {
                break;
            }

            if (alarmActive_)
            {
                if (logger_ != nullptr)
                {
                    logger_->log("ALARM THREAD: buzzer ON, LED RED, notify user. Reason = " + lastReason_);
                }
            }
            else
            {
                if (logger_ != nullptr)
                {
                    logger_->log("ALARM THREAD: buzzer OFF, LED NORMAL.");
                }
            }

            stateNotified_ = alarmActive_;
        }
    }

    bool stateChangedUnsafe() const
    {
        return alarmActive_ != stateNotified_;
    }

private:
    std::atomic<bool> running_{false};
    std::atomic<bool> alarmActive_{false};

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
    bool stateNotified_ = false;
    std::string lastReason_ = "unknown";
    AsyncLogger* logger_ = nullptr;
};

#endif