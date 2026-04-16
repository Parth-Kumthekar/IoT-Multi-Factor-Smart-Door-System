
#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream> //#inlcude <gpiod.h>
#ifdef __linux__
  #include <gpiod.h>
#else
  #include "include/gpiod_mock.h" 
#endif

class DoorLock {
public:
    // GPIO BCM pin 17 (physical pin 11) — active-HIGH relay
    static constexpr int  kGpioPin          = 17;
    static constexpr int  kUnlockDurationMs = 3000;  // door open time
    static constexpr int  kCooldownMs       = 5000;  // min between unlocks

    explicit DoorLock(int gpioPin = kGpioPin);
    ~DoorLock();

    // Non-copyable
    DoorLock(const DoorLock&)            = delete;
    DoorLock& operator=(const DoorLock&) = delete;

    // Trigger unlock if cooldown has elapsed; returns true if actually unlocked
    bool unlock(const std::string& personName);

    bool isLocked() const { return locked_.load(); }

private:
    void lockAfterDelay(int ms);

    int               gpioPin_;
    gpiod_chip*       chip_  = nullptr;
    gpiod_line*       line_  = nullptr;
    std::atomic<bool> locked_{true};
    std::chrono::steady_clock::time_point lastUnlock_;
    bool firstUnlock_ = true;
};