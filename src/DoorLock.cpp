#include "DoorLock.h"
#include "EventBus.h"
#include "AsyncLogger.h"
#include <iostream>

extern AsyncLogger gLogger;   

DoorController::DoorController() : relay_(kRelayPin, 0) {
    // TIMER THREAD
    timerThread_ = std::thread(&DoorController::timerLoop, this);

    // EventBus
    EventBus::instance().subscribe([this](const AccessEvent& ev){
        onAccessEvent(ev);
    });

    std::cout << "[Door] Controller ready, relay on BCM " << kRelayPin << '\n';
}

DoorController::~DoorController() {
    running_.store(false);
    timerCv_.notify_all();           // wake timer thread so it can exit
    if (timerThread_.joinable())
        timerThread_.join();         
    relay_.low();                    
}

void DoorController::onAccessEvent(const AccessEvent& ev) {
    if (ev.result == AuthResult::DENIED) return;   

    if (tryUnlock(ev.identity)) {
        gLogger.log(ev);             
    }
}

bool DoorController::tryUnlock(const std::string& identity) {
    auto now = std::chrono::steady_clock::now();
    if (!firstUnlock_) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
                  (now - lastUnlock_).count();
        if (ms < kCooldownMs) {
            std::cout << "[Door] Cooldown active (" << ms << "ms elapsed)\n";
            return false;
        }
    }

    firstUnlock_ = false;
    lastUnlock_  = now;
    state_.store(State::UNLOCKED);
    relay_.high();
    std::cout << "[Door] UNLOCKED for " << identity << '\n';

    // THE TIMER 
    {
        std::lock_guard<std::mutex> lk(timerMutex_);
        timerArmed_ = true;
    }
    timerCv_.notify_one();           
    return true;
}

void DoorController::timerLoop() {
    while (running_.load()) {
        std::unique_lock<std::mutex> lk(timerMutex_);
        
        timerCv_.wait(lk, [this]{
            return timerArmed_ || !running_.load();
        });
        if (!running_.load()) break;
        timerArmed_ = false;
        lk.unlock();

        
        std::this_thread::sleep_for(
            std::chrono::milliseconds(kUnlockDurationMs));

        relay_.low();
        state_.store(State::LOCKED);
        std::cout << "[Door] Re-locked after " << kUnlockDurationMs << "ms\n";
    }
}