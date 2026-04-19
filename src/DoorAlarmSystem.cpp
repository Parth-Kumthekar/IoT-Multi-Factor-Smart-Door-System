#include "DoorAlarmSystem.h"
#include <iostream>

DoorAlarmSystem::DoorAlarmSystem()
    : fsm_(alarmManager_, logger_)
{
}

DoorAlarmSystem::~DoorAlarmSystem()
{
    stop();
}

void DoorAlarmSystem::start()
{
    if (running_) return;

    // 1. Initialize Hardware first
    if (!nfcReader_.init()) {
        logger_.log("SYSTEM ERROR: NFC Reader failed to initialize on /dev/ttyAMA0");
    }

    // UPDATED: Using gpiochip0 based on your terminal gpiodetect results
    if (!outputController_.init()) {
        logger_.log("SYSTEM ERROR: Output Controller (GPIO) failed to initialize on gpiochip0");
    }

    running_ = true;

    // 2. Start Support Threads
    logger_.start();
    alarmManager_.start(logger_);

    // 3. Start Hardware Monitoring
    reedSwitch_.setCallback([this](int value) {
        onReedSwitchChange(value);
    });

    // UPDATED: Using chip 0 (pinctrl-rp1) for Raspberry Pi 5
    reedSwitch_.start(26, 0); 
    
    logger_.log("SYSTEM: Hardware mapping confirmed: Reed Switch on Pin 37 (GPIO 26).");
    logger_.log("SYSTEM: Started with Hardware Integration.");
    
    // Ensure FSM window is exactly 5 seconds
    fsm_.setAuthorizationWindow(std::chrono::milliseconds(5000));
    
    logger_.log("SYSTEM: FSM initial state = " + DoorAlarmFSM::toString(fsm_.getState()));

    // 4. Launch Internal Processing Threads
    nfcThread_ = std::thread(&DoorAlarmSystem::nfcLoop, this);
    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_ = std::thread(&DoorAlarmSystem::timerLoop, this);
}

void DoorAlarmSystem::stop()
{
    if (!running_) return;

    running_ = false;

    postEvent(EventType::Shutdown, "main");
    eventQueue_.shutdown();

    reedSwitch_.stop();

    if (nfcThread_.joinable())      nfcThread_.join();
    if (controlThread_.joinable())  controlThread_.join();
    if (timerThread_.joinable())    timerThread_.join();

    alarmManager_.stop();
    logger_.log("SYSTEM: Stopped.");
    logger_.stop();
}

void DoorAlarmSystem::postEvent(EventType type, const std::string& source)
{
    eventQueue_.push(Event(type, source));
}

void DoorAlarmSystem::onReedSwitchChange(int value) {
    // 1. Immediate Hardware Mirror (Visual Feedback)
    outputController_.setRedLed(value == 1);

    // 2. DEBOUNCE LOGIC:
    // Physical switches "bounce" (flicker) for a few milliseconds.
    // We wait 50ms to ensure the signal is stable before telling the FSM.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (value == 1) {
        logger_.log("DEBUG: Confirmed Door Open (1)");
        postEvent(EventType::DoorOpened, "ReedSwitch");
    } else {
        logger_.log("DEBUG: Confirmed Door Closed (0)");
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
            // Note: The FSM handleAuthorization will now check if the door is open.
            // If the door is open, the event will be ignored there.
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
            outputController_.granted(); // Green LED + Solenoid
        } else if (currentState == DoorAlarmFSM::State::AlarmActive) {
            // AlarmManager handles the buzzer, OutputController handles visual
            outputController_.denied();  
        }
    }
}

void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        // Check frequently (every 50ms) to see if time has run out
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        const auto state = fsm_.getState();
        const auto deadline = fsm_.getVerificationDeadline();

        // If we are waiting for a scan and the 5 seconds are up:
        if (state == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (std::chrono::steady_clock::now() >= deadline.value())
            {
                logger_.log("TIMER: Grace period expired. Triggering Timeout Event.");
                postEvent(EventType::VerificationTimeout, "timer");
                // Wait a bit to prevent event spam
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
}