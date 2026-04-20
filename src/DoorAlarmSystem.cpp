#include "DoorAlarmSystem.h"
#include <iostream>
#include <chrono>

DoorAlarmSystem::DoorAlarmSystem()
    : alarmManager_(outputController_), 
      fsm_(alarmManager_, logger_)
{
    // Use [this] capture to allow the hardware callback to access postEvent
    reedSwitch_.setCallback([this](int value) { 
        this->onReedSwitchChange(value); 
    });
}

DoorAlarmSystem::~DoorAlarmSystem() {
    stop();
}

/**
 * @brief Thread-safe bridge to the Event Queue.
 */
void DoorAlarmSystem::postEvent(EventType type, const std::string& data) {
    eventQueue_.push(Event(type, data));
}

void DoorAlarmSystem::start() {
    if (running_) return;

    logger_.start(); 
    alarmManager_.start(logger_);

    if (!nfcReader_.init()) logger_.log("ERROR", "NFC Hardware not found.");
    if (!outputController_.init()) logger_.log("ERROR", "GPIO Controller fail.");

    running_ = true;

    // Pin 26, Chip 4 is correct for Pi 5 physical header
    reedSwitch_.start(26, 4); 
    
    fsm_.setAuthorizationWindow(std::chrono::milliseconds(5000));

    // Dispatching the 4-pillar thread architecture
    nfcThread_     = std::thread(&DoorAlarmSystem::nfcLoop, this);
    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_   = std::thread(&DoorAlarmSystem::timerLoop, this);
    apiThread_     = std::thread(&DoorAlarmSystem::apiLoop, this);

    logger_.log("SYSTEM", "Logic Core Online. Initial State: " + DoorAlarmFSM::toString(fsm_.getState()));
}

void DoorAlarmSystem::stop() {
    if (!running_) return;
    running_ = false;

    // Stop the HTTP server first to unblock apiLoop
    svr_.stop();

    // Flush the event queue to unblock controlLoop
    postEvent(EventType::Shutdown, "system_stop");
    eventQueue_.shutdown();
    
    reedSwitch_.stop();

    // Join threads in reverse order of creation
    if (apiThread_.joinable())     apiThread_.join();
    if (timerThread_.joinable())   timerThread_.join();
    if (controlThread_.joinable()) controlThread_.join();
    if (nfcThread_.joinable())     nfcThread_.join();

    alarmManager_.stop();
    logger_.log("SYSTEM", "All threads joined safely.");
    logger_.stop();
}

void DoorAlarmSystem::nfcLoop() {
    while (running_) {
        std::string uid = nfcReader_.readUID();
        if (!uid.empty()) {
            if (accessController_.check(uid)) {
                postEvent(EventType::AuthorizedByNfc, uid);
            } else {
                logger_.log("AUTH", "Unauthorized Card Scanned.");
                outputController_.denied(); 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void DoorAlarmSystem::controlLoop() {
    while (running_) {
        Event event(EventType::PrintStatus);
        
        // Wait for event (Blocking call, 0% CPU usage while waiting)
        if (!eventQueue_.waitAndPop(event)) break;
        if (event.type == EventType::Shutdown) break;

        fsm_.handleEvent(event);

        // Hardware Feedback Sync
        auto state = fsm_.getState();
        if (state == DoorAlarmFSM::State::AuthorizedEntry) {
            outputController_.granted();
        } else if (state == DoorAlarmFSM::State::AlarmActive) {
            outputController_.denied();
        } else if (state == DoorAlarmFSM::State::ArmedIdle) {
            outputController_.setGreenLed(false);
            outputController_.setBuzzer(false);
            outputController_.setRedLed(false);
        }
    }
}

void DoorAlarmSystem::timerLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        auto deadline = fsm_.getVerificationDeadline();
        // Check if we are in pending state and if the current time has passed the deadline
        if (fsm_.getState() == DoorAlarmFSM::State::PendingVerification && deadline.has_value()) {
            if (std::chrono::steady_clock::now() >= deadline.value()) {
                logger_.log("SECURITY", "Grace period expired - Triggering Alarm.");
                postEvent(EventType::VerificationTimeout, "timer_service");
            }
        }
    }
}

void DoorAlarmSystem::apiLoop() {
    svr_.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        // Protect shared FSM data from concurrent access by the controlLoop
        std::lock_guard<std::mutex> lock(stateMtx_);
        
        auto state = fsm_.getState();
        std::string json = "{";
        json += "\"state\":\"" + DoorAlarmFSM::toString(state) + "\",";
        json += "\"doorOpen\":" + std::string(fsm_.isDoorOpen() ? "true" : "false");
        json += "}";
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(json, "application/json");
    });

    // Start the server (This blocks until svr_.stop() is called in stop())
    if (!svr_.listen("0.0.0.0", 3000)) {
        logger_.log("ERROR", "API Server failed to start on port 3000.");
    }
}

void DoorAlarmSystem::onReedSwitchChange(int value) {
    // 1 typically means "Open" for magnetic sensors, 0 means "Closed"
    if (value == 1) {
        postEvent(EventType::DoorOpened, "mag_sensor");
    } else {
        postEvent(EventType::DoorClosed, "mag_sensor");
    }
}