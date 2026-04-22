#include "DoorAlarmSystem.h"
#include <iostream>
#include <csignal>

/**
 * @brief Destroy the Door Alarm System.
 * @details Ensures a clean exit by joining background threads and releasing hardware.
 */
DoorAlarmSystem::~DoorAlarmSystem()
{
    stop();
}

/**
 * @brief Construct a new Door Alarm System.
 * @details Sets up dependency injection and binds hardware callbacks for sensors and buttons.
 */
DoorAlarmSystem::DoorAlarmSystem()
    : alarmManager_(outputController_), 
      fsm_(alarmManager_, logger_),
      cameraActive_(false) // Initialize camera state to false
{
    // Bind hardware callbacks
    reedSwitch_.setCallback([this](int value) { 
        this->onReedSwitchChange(value); 
    });

    exitButton_.setCallback([this](int value) {
        this->onButtonPress(value);
    });
}

/**
 * @brief Initializes hardware and spawns background service threads.
 * @details Raspberry Pi 5 uses gpiochip0. 
 * Pins: 23 (Reed), 19 (Button), 17 (External Camera Trigger).
 */
void DoorAlarmSystem::start()
{
    if (running_) return;

    if (!nfcReader_.init()) {
        logger_.log("SYSTEM ERROR: NFC Reader failed to initialize.");
    }

    if (!outputController_.init()) {
        logger_.log("CRITICAL ERROR: Output Controller failed on gpiochip0.");
        return; 
    }

    running_ = true;
    logger_.start();
    alarmManager_.start(logger_);

    // Start hardware monitoring
    reedSwitch_.start(23, 0); 
    exitButton_.start(19, 0);
    cameraTrigger_.start(17, 0); // Pin connected to Camera Block Output
    
    // Set callback for External Camera Signal
    cameraTrigger_.setCallback([this](int value) {
        bool signalHigh = (value == 1);
        if (signalHigh != this->cameraActive_) {
            this->cameraActive_ = signalHigh;
            if (this->cameraActive_) {
                logger_.log("EXTERNAL: Camera signal detected (HIGH).");
            } else {
                logger_.log("EXTERNAL: Camera signal lost (LOW).");
            }
            // Recalculate Green LED state immediately when camera signal changes
            this->updateGreenLedLogic();
        }
    });

    fsm_.setAuthorizationWindow(std::chrono::milliseconds(5000));
    logger_.log("SYSTEM: Initial State = " + DoorAlarmFSM::toString(fsm_.getState()));

    nfcThread_ = std::thread(&DoorAlarmSystem::nfcLoop, this);
    controlThread_ = std::thread(&DoorAlarmSystem::controlLoop, this);
    timerThread_ = std::thread(&DoorAlarmSystem::timerLoop, this);
    apiThread_ = std::thread(&DoorAlarmSystem::apiLoop, this);

    logger_.log("SYSTEM: All threads active.");
}

/**
 * @brief Performs a synchronized shutdown of the entire system.
 */
void DoorAlarmSystem::stop()
{
    if (!running_) return;
    running_ = false;

    svr_.stop();
    postEvent(EventType::Shutdown, "main");
    eventQueue_.shutdown();
    reedSwitch_.stop();
    exitButton_.stop();
    cameraTrigger_.stop();

    if (nfcThread_.joinable())   nfcThread_.join();
    if (controlThread_.joinable()) controlThread_.join();
    if (timerThread_.joinable())   timerThread_.join();
    if (apiThread_.joinable())     apiThread_.join(); 

    alarmManager_.stop();
    logger_.log("SYSTEM: Stopped.");
    logger_.stop();
}

/**
 * @brief Centralized "OR Gate" logic for the Green LED.
 * @details Logic: (FSM is Authorized AND Door is Closed) OR (Camera signal is HIGH).
 */
void DoorAlarmSystem::updateGreenLedLogic() {
    bool fsmAuthorized = (fsm_.getState() == DoorAlarmFSM::State::AuthorizedEntry);
    bool doorClosed = !fsm_.isDoorOpen();

    // SOFTWARE OR GATE
    if ((fsmAuthorized && doorClosed) || cameraActive_) {
        outputController_.setGreenLed(true);
    } else {
        outputController_.setGreenLed(false);
    }
}

/**
 * @brief GPIO Callback handler for the reed switch.
 * @param value The physical state (1 = Open, 0 = Closed).
 */
void DoorAlarmSystem::onReedSwitchChange(int value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool isOpen = (value == 1); 

    fsm_.setDoorState(isOpen);

    // Refresh LED state because door status changed
    updateGreenLedLogic();

    if (isOpen) {
        outputController_.setRedLed(true); 
        logger_.log("SENSOR: Door physically OPENED.");
        postEvent(EventType::DoorOpened, "ReedSwitch");
    } else {
        outputController_.setRedLed(false);
        logger_.log("SENSOR: Door physically CLOSED.");
        postEvent(EventType::DoorClosed, "ReedSwitch");
    }
}

/**
 * @brief Logic for the manual exit push button.
 */
void DoorAlarmSystem::onButtonPress(int value) {
    static auto lastPressTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (value == 1 && (now - lastPressTime > std::chrono::milliseconds(500))) { 
        if (fsm_.isDoorOpen()) {
            logger_.log("HARDWARE: Exit button ignored. Door is already open.");
            return;
        }
        lastPressTime = now; 
        logger_.log("HARDWARE: Exit button pressed.");
        postEvent(EventType::AuthorizedByApp, "Push_Button");
    }
}

/**
 * @brief Background thread for polling the NFC reader.
 */
void DoorAlarmSystem::nfcLoop()
{
    while (running_)
    {
        std::string uid = nfcReader_.readUID();
        if (!uid.empty())
        {
            if (fsm_.isDoorOpen()) {
                logger_.log("NFC: Tag " + uid + " ignored. Door is open.");
            } else {
                logger_.log("NFC: Detected UID " + uid);
                if (accessController_.check(uid)) {
                    postEvent(EventType::AuthorizedByNfc, uid);
                } else {
                    logger_.log("NFC: Access Denied for UID " + uid);
                    std::thread([this]() {
                        outputController_.setBuzzer(true);
                        outputController_.setRedLed(true);
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        outputController_.setBuzzer(false);
                        outputController_.setRedLed(false);
                    }).detach();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

/**
 * @brief Core logic consumer thread.
 */
void DoorAlarmSystem::controlLoop()
{
    while (true)
    {
        Event event(EventType::PrintStatus);
        if (!eventQueue_.waitAndPop(event)) break;
        if (event.type == EventType::Shutdown) break;

        fsm_.handleEvent(event);

        // Every FSM event could change the authorization state, so refresh LEDs
        updateGreenLedLogic();

        if (fsm_.getState() == DoorAlarmFSM::State::AlarmActive) {
            outputController_.denied();  
        }
    }
}

/**
 * @brief Background thread for managing temporal logic and auto-reset.
 */
void DoorAlarmSystem::timerLoop()
{
    DoorAlarmFSM::State lastState = DoorAlarmFSM::State::ArmedIdle;
    auto stateStartTime = std::chrono::steady_clock::now();

    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        const auto currentState = fsm_.getState();

        if (currentState != lastState) {
            stateStartTime = std::chrono::steady_clock::now();
            lastState = currentState;
        }

        auto elapsed = std::chrono::steady_clock::now() - stateStartTime;

        // 1. Auto-silence Alarm (5s)
        if (currentState == DoorAlarmFSM::State::AlarmActive && elapsed > std::chrono::seconds(5)) {
            alarmManager_.clearAlarm(); 
        }

        // 2. Auto-Reset Green LED (1s) - This resets FSM state
        if (currentState == DoorAlarmFSM::State::AuthorizedEntry && elapsed > std::chrono::seconds(1)) {
            logger_.log("TIMER: Authorized window expired. Resetting FSM.");
            postEvent(EventType::DoorClosed, "AutoReset"); 
            // Note: updateGreenLedLogic() will be called in controlLoop when this event is processed
        }

        // 3. Handle Grace Period Timeout
        const auto deadline = fsm_.getVerificationDeadline();
        if (currentState == DoorAlarmFSM::State::PendingVerification && deadline.has_value())
        {
            if (std::chrono::steady_clock::now() >= deadline.value())
            {
                postEvent(EventType::VerificationTimeout, "timer");
            }
        }
    }
}

/**
 * @brief Background thread for the Web Dashboard API.
 */
void DoorAlarmSystem::apiLoop() {
    try {
        svr_.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
            res.set_content("State: " + DoorAlarmFSM::toString(fsm_.getState()), "text/plain");
        });
        svr_.listen("0.0.0.0", 3000);
    } catch (...) {}
}

/**
 * @brief Internal helper to push events into the thread-safe queue.
 */
void DoorAlarmSystem::postEvent(EventType type, const std::string& source) {
    eventQueue_.push(Event(type, source));
}
