#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <string>
#include "GpioPin.h"
#include "AccessEvent.h"

class DoorController {
public:
    
    static constexpr unsigned int kRelayPin        = 17; // pin for the solenoid 
    static constexpr int          kUnlockDurationMs = 3000;
    static constexpr int          kCooldownMs       = 5000;

    DoorController();
    ~DoorController();                   

    DoorController(const DoorController&)            = delete;
    DoorController& operator=(const DoorController&) = delete;

    // Called by EventBus 
    void onAccessEvent(const AccessEvent& ev);

    bool isLocked()   const { return state_ == State::LOCKED; }
    bool isUnlocked() const { return state_ == State::UNLOCKED; }

private:
    enum class State { LOCKED, UNLOCKED };

    // Attempt unlock 
    bool tryUnlock(const std::string& identity);

    // TIMER THREAD
    void timerLoop();

    GpioPin                               relay_;
    std::atomic<State>                    state_{State::LOCKED};
    std::chrono::steady_clock::time_point lastUnlock_;
    bool                                  firstUnlock_{true};
    std::mutex                            timerMutex_;
    std::condition_variable               timerCv_;
    bool                                  timerArmed_{false};
    std::thread                           timerThread_;
    std::atomic<bool>                     running_{true};
};