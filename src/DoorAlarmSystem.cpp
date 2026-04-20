#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>

DoorAlarmSystem::~DoorAlarmSystem()
{
    stop();
}
DoorAlarmSystem::DoorAlarmSystem()
    : alarmManager_(outputController_), 
      fsm_(alarmManager_, logger_)
{
    // Configure hardware callbacks
    reedSwitch_.setCallback([this](int value) { 
        this->onReedSwitchChange(value); 
    });
}
void DoorAlarmSystem::start()
{
    if (running_) return;

    // 1. Initialize Hardware
    if (!nfcReader_.init()) {
        logger_.log("SYSTEM ERROR: NFC Reader failed to initialize on /dev/ttyAMA0");
    }

    if (!outputController_.init()) {
        logger_.log("SYSTEM ERROR: Output Controller (GPIO) failed to initialize on gpiochip0");
    }

    running_ = true;

    // 2. Start Support Threads
    logger_.start();             // Note: Ensure AsyncLogger is updated to save to a .csv file!
    alarmManager_.start(logger_); // Note: Ensure AlarmManager triggers the Email thread!

    // 3. Start Hardware Monitoring
    reedSwitch_.setCallback([this](int value) {
        onReedSwitchChange(value);
    });

    reedSwitch_.start(26, 0); 
    
    // Set 5-second window
    fsm_.setAuthorizationWindow(std::chrono::milliseconds(5000));
    
    logger_.log("SYSTEM: Initial State = " + DoorAlarmFSM::toString(fsm_.getState()));

    // 4. Launch Internal Processing Threads
    nfcThread_ = std::thread(&DoorAlarmSystem::nfcLoop, this);
    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_ = std::thread(&DoorAlarmSystem::timerLoop, this);

    // 5. NEW: Launch Web Dashboard API Thread
    apiThread_ = std::thread(&DoorAlarmSystem::apiLoop, this);

    logger_.log("SYSTEM: Web Dashboard API started on port 3000.");
    logger_.log("SYSTEM: All threads active.");
}

void DoorAlarmSystem::stop()
{
    if (!running_) return;

    running_ = false;

    // Stop the Web Server first
    svr_.stop();

    postEvent(EventType::Shutdown, "main");
    eventQueue_.shutdown();

    reedSwitch_.stop();

    // Join all threads
    if (nfcThread_.joinable())      nfcThread_.join();
    if (controlThread_.joinable())  controlThread_.join();
    if (timerThread_.joinable())    timerThread_.join();
    if (apiThread_.joinable())      apiThread_.join(); // Join the API thread

    alarmManager_.stop();
    logger_.log("SYSTEM: Stopped.");
    logger_.stop();
}

// --- NEW: WEB DASHBOARD API IMPLEMENTATION ---
void DoorAlarmSystem::apiLoop()
{
    // Endpoint for Dashboard Status
    svr_.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        // Protect state access if your FSM isn't thread-safe for reads
        std::lock_guard<std::mutex> lock(stateMtx_);
        
        auto state = fsm_.getState();
        
        // Build JSON Response
        std::string json = "{";
        json += "\"ok\":true,";
        json += "\"state\":\"" + DoorAlarmFSM::toString(state) + "\",";
        json += "\"alarmActive\":" + std::string(state == DoorAlarmFSM::State::AlarmActive ? "true" : "false") + ",";
json += "\"doorOpen\":" + std::string(fsm_.isDoorOpen() ? "true" : "false");
        json += "}";

        res.set_header("Access-Control-Allow-Origin", "*"); // Allow Web Dashboard Access
        res.set_content(json, "application/json");
    });

    // Endpoint for Manual Alarm Clear from Dashboard
    svr_.Post("/api/alarm/clear", [this](const httplib::Request&, httplib::Response& res) {
        logger_.log("WEB: Remote alarm clear requested.");
        // If your FSM doesn't have a clear event, you might need to add one
        // postEvent(EventType::ManualReset, "web_dashboard"); 
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("{\"ok\":true}", "application/json");
    });

    if (!svr_.listen("0.0.0.0", 3000)) {
        logger_.log("SYSTEM ERROR: Web Server failed to bind to port 3000");
    }
}

void DoorAlarmSystem::postEvent(EventType type, const std::string& source)
{
    eventQueue_.push(Event(type, source));
}


void DoorAlarmSystem::onReedSwitchChange(int value) {
    // Debounce: Wait for mechanical vibration to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Re-read or trust the value to set the indicator LED
    bool isOpen = (value == 1); 
    outputController_.setRedLed(isOpen);

    if (isOpen) {
        logger_.log("SENSOR: Door contact broken (Open)");
        postEvent(EventType::DoorOpened, "ReedSwitch");
    } else {
        logger_.log("SENSOR: Door contact established (Closed)");
        postEvent(EventType::DoorClosed, "ReedSwitch");
    }
}
void DoorAlarmSystem::nfcLoop()
{
    while (running_)
    {
        std::string uid = nfcReader_.readUID();
        if (!uid.empty())
        {
            logger_.log("NFC THREAD: Detected UID " + uid);
            if (accessController_.check(uid)) {
                postEvent(EventType::AuthorizedByNfc, uid);
            } else {
                logger_.log("NFC THREAD: Access Denied for UID " + uid);
                outputController_.denied(); 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void DoorAlarmSystem::controlLoop()
{
    while (true)
    {
        Event event(EventType::PrintStatus);
        if (!eventQueue_.waitAndPop(event)) break;

        if (event.type == EventType::Shutdown) {
            logger_.log("CONTROL: Shutdown event received.");
            break;
        }

        fsm_.handleEvent(event);

        auto currentState = fsm_.getState();
        if (currentState == DoorAlarmFSM::State::AuthorizedEntry) {
            outputController_.granted(); 
        } else if (currentState == DoorAlarmFSM::State::AlarmActive) {
            outputController_.denied();  
        }
    }
}

void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const auto state = fsm_.getState();
        const auto deadline = fsm_.getVerificationDeadline();

        if (state == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (std::chrono::steady_clock::now() >= deadline.value())
            {
                logger_.log("TIMER: Grace period expired. Triggering Timeout Event.");
                postEvent(EventType::VerificationTimeout, "timer");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
}
