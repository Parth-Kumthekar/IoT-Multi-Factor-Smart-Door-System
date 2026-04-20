#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

// Include your logic components
#include "DoorAlarmFSM.h"
#include "AlarmManager.h"
#include "AsyncLogger.h"
#include "OutputController.hpp" // Mocked or actual
#include "EventQueue.h"

/**
 * @brief Test Case 1: The "Happy Path"
 * Scenario: System is armed, door opens, user taps NFC within 5 seconds.
 * Expected: Alarm remains OFF, state transitions to AuthorizedEntry.
 */
void test_successful_entry() {
    OutputController oc; 
    AsyncLogger logger;
    AlarmManager am(oc);
    DoorAlarmFSM fsm(am, logger);

    // Initial State should be ArmedIdle
    assert(fsm.getState() == DoorAlarmFSM::State::ArmedIdle);

    // 1. Open the door
    fsm.handleEvent({EventType::DoorOpened, "Sensor"});
    assert(fsm.getState() == DoorAlarmFSM::State::PendingVerification);

    // 2. Immediate NFC tap
    fsm.handleEvent({EventType::AuthorizedByNfc, "UID_123"});
    assert(fsm.getState() == DoorAlarmFSM::State::AuthorizedEntry);
    assert(am.isAlarmActive() == false);

    std::cout << "[PASS] Test 1: Successful Entry" << std::endl;
}

/**
 * @brief Test Case 2: Security Breach (Timeout)
 * Scenario: System is armed, door opens, no verification occurs.
 * Expected: State becomes AlarmActive, AlarmManager triggers buzzer.
 */
void test_security_breach_timeout() {
    OutputController oc;
    AsyncLogger logger;
    AlarmManager am(oc);
    DoorAlarmFSM fsm(am, logger);

    fsm.handleEvent({EventType::DoorOpened, "Sensor"});
    
    // Simulate the Timer Thread detecting a timeout
    fsm.handleEvent({EventType::VerificationTimeout, "Timer"});

    assert(fsm.getState() == DoorAlarmFSM::State::AlarmActive);
    assert(am.isAlarmActive() == true);

    std::cout << "[PASS] Test 2: Security Breach Timeout" << std::endl;
}

/**
 * @brief Test Case 3: Auto-Rearm
 * Scenario: Door was open and authorized, then door is closed.
 * Expected: System automatically returns to ArmedIdle.
 */
void test_auto_rearm() {
    OutputController oc;
    AsyncLogger logger;
    AlarmManager am(oc);
    DoorAlarmFSM fsm(am, logger);

    fsm.handleEvent({EventType::DoorOpened, "Sensor"});
    fsm.handleEvent({EventType::AuthorizedByNfc, "UID_123"});
    
    // Door closes
    fsm.handleEvent({EventType::DoorClosed, "Sensor"});

    assert(fsm.getState() == DoorAlarmFSM::State::ArmedIdle);
    assert(am.isAlarmActive() == false);

    std::cout << "[PASS] Test 3: Auto-Rearm Logic" << std::endl;
}


/**
 * @brief Stress Test: Rapid Event Flooding
 */
void test_event_queue_concurrency() {
    EventQueue queue;
    
    // Producer: Rapidly push 100 events
    std::thread producer([&queue]() {
        for(int i=0; i<100; ++i) {
            queue.push({EventType::PrintStatus, "StressTest"});
        }
    });

    // Consumer: Try to pop them
    int count = 0;
    while(count < 100) {
        Event e(EventType::PrintStatus); // Provide a default type for testing
        if(queue.waitAndPop(e)) count++;
    }

    producer.join();
    assert(count == 100);
    std::cout << "[PASS] Test 4: Event Queue Concurrency" << std::endl;
}

int main() {
    std::cout << "--- STARTING LOGIC UNIT TESTS ---\n";
    

    test_successful_entry();
    test_security_breach_timeout();
    test_auto_rearm();
    std::cout << "Running Concurrency Tests..." << std::endl;
    test_event_queue_concurrency();

    std::cout << "--- ALL TESTS PASSED ---\n";
    return 0;
}


