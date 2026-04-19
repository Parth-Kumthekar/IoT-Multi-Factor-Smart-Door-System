#include "DoorAlarmSystem.h"
#include <iostream>

DoorAlarmSystem::DoorAlarmSystem()
    : fsm_(alarmManager_, logger_)
{
    // Note: Initialization of nfcReader_, etc., happens via their default constructors
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

    if (!outputController_.init()) {
        logger_.log("SYSTEM ERROR: Output Controller (GPIO) failed to initialize on gpiochip4");
    }

    running_ = true;

    // 2. Start Support Threads
    logger_.start();
    alarmManager_.start(logger_);

    // 3. Start Hardware Monitoring
    reedSwitch_.setCallback([this](int value) {
        onReedSwitchChange(value);
    });

    // FIX: start() returns void, so we call it directly without the 'if' check
    reedSwitch_.start(26, 4); 
    
    logger_.log("SYSTEM: Hardware mapping confirmed: Reed Switch on Pin 37 (GPIO 26).");
    logger_.log("SYSTEM: Started with Hardware Integration.");
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

    // Trigger shutdown for the event queue
    postEvent(EventType::Shutdown, "main");
    eventQueue_.shutdown();

    // Stop Hardware listeners
    reedSwitch_.stop();

    // Join all threads
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
    // 1. Hardware Mirror: Pin 37 (Input GPIO 26) -> Pin 11 (Output GPIO 17)
    // This allows the Red LED to follow the door status immediately.
    outputController_.setRedLed(value == 1);

    // 2. FSM Logic
    if (value == 1) {
        logger_.log("DEBUG: Door Opened (1) -> Red LED ON");
        postEvent(EventType::DoorOpened, "ReedSwitch");
    } else {
        logger_.log("DEBUG: Door Closed (0) -> Red LED OFF");
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

            // Cross-reference with the Access Controller
            if (accessController_.check(uid)) {
                postEvent(EventType::AuthorizedByNfc, uid);
            } else {
                logger_.log("NFC THREAD: Access Denied for UID " + uid);
                outputController_.denied(); // Immediate visual feedback
            }
        }

        // Small sleep to prevent 100% CPU usage during polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

        // Pass event to FSM
        fsm_.handleEvent(event);

        // React to State Changes for Hardware Outputs
        auto currentState = fsm_.getState();
        if (currentState == DoorAlarmFSM::State::AuthorizedEntry) {
            outputController_.granted(); // Open Solenoid, Green LED
        } else if (currentState == DoorAlarmFSM::State::AlarmActive) {
            outputController_.denied();  // Red LED, Buzzer
        }
    }
}

void DoorAlarmSystem::timerLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(Ms(50));

        const auto state = fsm_.getState();
        const auto deadline = fsm_.getVerificationDeadline();

        if (state == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (Clock::now() >= deadline.value())
            {
                postEvent(EventType::VerificationTimeout, "timer");
                std::this_thread::sleep_for(Ms(100));
            }
        }
    }
}