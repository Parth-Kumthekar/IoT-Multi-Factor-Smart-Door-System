#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>

/**
 * @brief Constructor handles dependency injection and callback registration.
 */
DoorAlarmSystem::DoorAlarmSystem()
    : alarmManager_(outputController_), 
      fsm_(alarmManager_, logger_)
{
    // Register the interrupt handler for the magnetic reed switch
    reedSwitch_.setCallback([this](int value) { 
        this->onReedSwitchChange(value); 
    });
}

DoorAlarmSystem::~DoorAlarmSystem()
{
    stop();
}

/**
 * @brief Initialization sequence for hardware and system threads.
 * * This method ensures all hardware is claimed before launching the 
 * concurrent processing loops.
 */
void DoorAlarmSystem::start()
{
    if (running_) return;

    // 1. Hardware Initialization (Fail-safe checks)
    if (!nfcReader_.init()) {
        logger_.log("CRITICAL", "NFC Reader failed to initialize on /dev/ttyAMA0");
    }

    if (!outputController_.init()) {
        logger_.log("CRITICAL", "Output Controller (GPIO) failed to initialize.");
    }

    running_ = true;

    // 2. Service Activation
    logger_.start(); 
    alarmManager_.start(logger_);

    // 3. Hardware Monitoring
    // Reed switch on BCM Pin 26, using gpiochip0
    reedSwitch_.start(26, 0); 
    
    fsm_.setAuthorizationWindow(std::chrono::milliseconds(5000));
    logger_.log("SYSTEM", "Initial State = " + DoorAlarmFSM::toString(fsm_.getState()));

    // 4. Thread Dispatching (Producer-Consumer Architecture)
    // Each thread manages a specific domain of the system
    nfcThread_     = std::thread(&DoorAlarmSystem::nfcLoop, this);
    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_   = std::thread(&DoorAlarmSystem::timerLoop, this);
    apiThread_     = std::thread(&DoorAlarmSystem::apiLoop, this);

    logger_.log("SYSTEM", "Multi-threaded logic active. Web API on port 3000.");
}

/**
 * @brief Graceful shutdown procedure.
 * * Ensures the Web Server and hardware threads are joined before 
 * releasing system resources.
 */
void DoorAlarmSystem::stop()
{
    if (!running_) return;

    running_ = false;

    // Shutdown the blocking HTTP server
    svr_.stop();

    // Signal the event queue to flush and close
    postEvent(EventType::Shutdown, "main");
    eventQueue_.shutdown();

    reedSwitch_.stop();

    // Thread Join sequence (LIFO)
    if (nfcThread_.joinable())      nfcThread_.join();
    if (controlThread_.joinable())  controlThread_.join();
    if (timerThread_.joinable())    timerThread_.join();
    if (apiThread_.joinable())      apiThread_.join();

    alarmManager_.stop();
    logger_.log("SYSTEM", "Graceful shutdown complete.");
    logger_.stop();
}

/**
 * @brief REST API Loop for Remote Dashboard Integration.
 * * Provides a JSON endpoint for system status and remote controls.
 */
void DoorAlarmSystem::apiLoop()
{
    // Status Endpoint: Used by Web Frontend to visualize system state
    svr_.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(stateMtx_);
        
        auto state = fsm_.getState();
        
        // Manual JSON construction (Use a library like nlohmann/json for A1 production)
        std::string json = "{";
        json += "\"ok\":true,";
        json += "\"state\":\"" + DoorAlarmFSM::toString(state) + "\",";
        json += "\"alarmActive\":" + std::string(state == DoorAlarmFSM::State::AlarmActive ? "true" : "false") + ",";
        json += "\"doorOpen\":" + std::string(fsm_.isDoorOpen() ? "true" : "false");
        json += "}";

        res.set_header("Access-Control-Allow-Origin", "*"); 
        res.set_content(json, "application/json");
    });

    // Remote Command: Silences alarm via web dashboard
    svr_.Post("/api/alarm/clear", [this](const httplib::Request&, httplib::Response& res) {
        logger_.log("WEB", "Remote alarm clear request received.");
        postEvent(EventType::DisarmSystem, "web_dashboard"); 
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("{\"ok\":true}", "application/json");
    });

    if (!svr_.listen("0.0.0.0", 3000)) {
        logger_.log("ERROR", "Web Server failed to bind to port 3000.");
    }
}

/**
 * @brief ISR-style callback for the Reed Switch.
 * * High-priority response to physical door changes.
 */
void DoorAlarmSystem::onReedSwitchChange(int value) {
    // Debounce to handle mechanical contact bounce
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    bool isOpen = (value == 1); 
    outputController_.setRedLed(isOpen);

    if (isOpen) {
        logger_.log("SENSOR", "Door contact broken (Open)");
        postEvent(EventType::DoorOpened, "ReedSwitch");
    } else {
        logger_.log("SENSOR", "Door contact established (Closed)");
        postEvent(EventType::DoorClosed, "ReedSwitch");
    }
}

/**
 * @brief Dedicated loop for serial NFC hardware monitoring.
 */
void DoorAlarmSystem::nfcLoop()
{
    while (running_)
    {
        std::string uid = nfcReader_.readUID();
        if (!uid.empty())
        {
            if (accessController_.check(uid)) {
                postEvent(EventType::AuthorizedByNfc, uid);
            } else {
                logger_.log("ACCESS", "Unauthorized UID: " + uid);
                outputController_.denied(); 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

/**
 * @brief Primary Control Loop (Consumer).
 * * Consumes events from the thread-safe queue and drives the FSM.
 */
void DoorAlarmSystem::controlLoop()
{
    while (true)
    {
        Event event(EventType::PrintStatus);
        // Blocking wait: No CPU usage while the queue is empty
        if (!eventQueue_.waitAndPop(event)) break;

        if (event.type == EventType::Shutdown) break;

        fsm_.handleEvent(event);

        // Hardware Feedback synchronization
        auto currentState = fsm_.getState();
        if (currentState == DoorAlarmFSM::State::AuthorizedEntry) {
            outputController_.granted(); 
        } else if (currentState == DoorAlarmFSM::State::AlarmActive) {
            outputController_.denied();  
        }
    }
}

/**
 * @brief Temporal Monitoring Loop.
 * * Manages grace periods and system timeouts.
 */
void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        const auto state = fsm_.getState();
        const auto deadline = fsm_.getVerificationDeadline();

        if (state == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (std::chrono::steady_clock::now() >= deadline.value())
            {
                logger_.log("TIMER", "Grace period expired.");
                postEvent(EventType::VerificationTimeout, "timer");
                // Brief sleep to prevent event flooding
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
}