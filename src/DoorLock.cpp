#include "DoorLock.h"
#include "EventBus.h"
#include "Logger.h"
#include <iostream>

extern AsyncLogger gLogger;   

DoorController::DoorController() : relay_(kRelayPin, 0),
      greenLed_(kGreenLedPin, 0),
      redLed_(kRedLedPin, 0){
    // TIMER THREAD
    timerThread_ = std::thread(&DoorController::timerLoop, this);

    // EventBus
    EventBus::instance().subscribe([this](const AccessEvent& ev){
        onAccessEvent(ev);
    });

    std::cout << " Controller ready, relay on BCM " << kRelayPin << '\n';
}

DoorController::~DoorController() {
    running_.store(false);
    timerCv_.notify_all();           // wake timer thread so it can exit
    if (timerThread_.joinable())
        timerThread_.join();         
    relay_.low();                    
}

void DoorController::onAccessEvent(const AccessEvent& ev) {
    if (ev.result == AuthResult::DENIED){
        redLed_.high();   // turn ON red LED
        greenLed_.low();  // ensure green OFF

        std::cout << "[Door] Access denied\n";

        // turn OFF red LED after short delay
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            redLed_.low();
        }).detach();
     return; 
    }  
    redLed_.low();
    if (tryUnlock(ev.identity)) {
        greenLed_.high(); // turn ON green LED
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

        //  LOCK DOOR
        relay_.low();
        state_.store(State::LOCKED);

        // RESET LEDs
        greenLed_.low();
        redLed_.low();

        std::cout << "[Door] Re-locked\n";
    }
}