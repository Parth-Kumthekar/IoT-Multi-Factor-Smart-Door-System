# Door Alarm Thread Design Project

## Overview
This project is a simplified **door access alarm system** implemented in **C++17**.  
It demonstrates how a multi-threaded event-driven architecture can be used to manage door security logic, user authorization, and alarm handling in a clean and modular way.

The system is built around three core ideas:

- **Thread Design**: separate threads handle control logic, timer checking, alarm output, and asynchronous logging.
- **Event Queue**: all system actions are converted into events and processed in a thread-safe queue.
- **Finite State Machine (FSM)**: the door alarm behaviour is controlled by explicit system states and transitions.

This project is suitable as a course demonstration of **concurrency design**, **event-driven programming**, and **FSM-based control logic** in an embedded or Raspberry Pi style security system.

---

## Main Features
- Arm and disarm the security system
- Detect simulated door open and close events
- Support authorization by **NFC** or **APP**
- Trigger alarm when the door is opened without valid authorization
- Use a **verification timeout window** before raising the alarm
- Print asynchronous system logs with timestamps
- Demonstrate safe communication between threads using a shared event queue

---

## System Architecture

### 1. Thread Design
The system uses multiple threads to separate responsibilities:

- **Control Thread**  
  Continuously consumes events from the event queue and sends them to the FSM for processing.

- **Timer Thread**  
  Periodically checks whether the authorization window has expired.  
  If timeout happens, it posts a `VerificationTimeout` event to the queue.

- **Alarm Thread**  
  Managed by `AlarmManager`, responsible for simulating alarm actions such as buzzer, red LED, and user notification.

- **Logger Thread**  
  Managed by `AsyncLogger`, prints system logs asynchronously so that logging does not block the main control flow.

This design improves modularity and makes the system easier to extend for real hardware integration.

---

### 2. Event Queue
The `EventQueue` class is the core communication mechanism between threads.

It provides:
- thread-safe event insertion
- blocking event waiting
- timed waiting
- non-blocking pop
- clean shutdown support

All actions in the system are represented as events, such as:
- `ArmSystem`
- `DisarmSystem`
- `DoorOpened`
- `DoorClosed`
- `AuthorizedByNfc`
- `AuthorizedByApp`
- `VerificationTimeout`
- `PrintStatus`
- `Shutdown`

This event-driven approach reduces direct coupling between modules and makes the control flow clearer.

---

### 3. Finite State Machine (FSM)
The `DoorAlarmFSM` class manages the security logic using explicit states.

#### FSM States
- `Disarmed`  
  System is inactive. Door opening is allowed.

- `ArmedIdle`  
  System is armed and waiting for events.

- `PendingVerification`  
  Door has opened while armed, and the system is waiting for valid authorization within a limited time window.

- `AuthorizedEntry`  
  Valid authorization has been received, so entry is temporarily allowed.

- `AlarmActive`  
  No valid authorization was received before timeout, so the alarm is triggered.

- `Fault`  
  Reserved for future fault-handling extension.

#### Example Transition Logic
- `Disarmed -> ArmedIdle` when the system is armed
- `ArmedIdle -> PendingVerification` when the door opens
- `PendingVerification -> AuthorizedEntry` when NFC/APP authorization succeeds in time
- `PendingVerification -> AlarmActive` when verification times out
- `AuthorizedEntry -> ArmedIdle` when the door is closed
- any state -> `Disarmed` when the system is disarmed

This makes the security logic explicit, readable, and easy to test.

---

## Project Structure
```text
.
├── CMakeLists.txt
├── include/
│   ├── AlarmManager.h
│   ├── AsyncLogger.h
│   ├── DoorAlarmFSM.h
│   ├── DoorAlarmSystem.h
│   ├── Event.h
│   └── EventQueue.h
└── src/
    ├── DoorAlarmFSM.cpp
    ├── DoorAlarmSystem.cpp
    └── main.cpp

DoorAlarmSystem

Top-level controller of the whole system.
It:

starts and stops all components
owns the event queue
launches the control thread and timer thread
forwards events into the system
DoorAlarmFSM

Implements the system state machine.
It:

handles all incoming events
manages state transitions
controls the authorization window
interacts with the alarm manager and logger
EventQueue

A thread-safe queue for inter-thread communication.
It ensures that events are processed safely and in order.

AlarmManager

Runs a dedicated alarm thread.
It simulates alarm behaviour such as:

buzzer ON/OFF
LED status
user notification logging
AsyncLogger

Runs a dedicated logging thread and prints timestamped messages asynchronously.

Build Instructions
Requirements
C++17 compatible compiler
CMake 3.16 or above
Build
mkdir build
cd build
cmake ..
cmake --build .
Run
./door_alarm

On Windows:

door_alarm.exe
Console Commands

After running the program, you can enter the following commands:

arm        -> arm system
disarm     -> disarm system
door_open  -> simulate door open
door_close -> simulate door close
nfc_ok     -> simulate valid NFC auth
app_ok     -> simulate valid APP auth
status     -> print system status
help       -> show commands
quit       -> exit program
Example Test Flow
Authorized Access
arm
door_open
nfc_ok
door_close

Expected result:

system enters PendingVerification
valid NFC authorization is accepted
state changes to AuthorizedEntry
after door closes, system returns to ArmedIdle
no alarm is triggered
Unauthorized Access
arm
door_open
wait until timeout

Expected result:

system enters PendingVerification
no valid authorization is received
timeout event is generated
state changes to AlarmActive
alarm thread reports buzzer/LED warning
