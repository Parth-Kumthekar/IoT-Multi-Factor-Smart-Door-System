#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

class AsyncLogger
{
public:
    AsyncLogger() = default;

    ~AsyncLogger()
    {
        stop();
    }

    void start()
    {
        if (running_)
        {
            return;
        }

        running_ = true;
        thread_ = std::thread(&AsyncLogger::run, this);
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

    void log(const std::string& message)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.push("[" + nowString() + "] " + message);
        }
        cv_.notify_one();
    }

private:
    static std::string nowString()
    {
        auto t = std::time(nullptr);
        std::tm tm {};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

    void run()
    {
        while (running_ || !messages_.empty())
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return !running_ || !messages_.empty(); });

            while (!messages_.empty())
            {
                std::cout << messages_.front() << std::endl;
                messages_.pop();
            }
        }
    }

private:
    std::atomic<bool> running_ {false};
    std::queue<std::string> messages_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
};

#endif