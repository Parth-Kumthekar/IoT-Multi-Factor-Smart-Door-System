#  IoT Multi-Factor Smart Door System — Raspberry Pi 5

This project presents a secure smart door access system using IoT and multi-factor authentication. It combines multiple verification methods such as RFID, and faical recognition to ensure only authorized users can unlock the door. The system is built using raspberry pi and connected modules, enabling real-time monitoring and remote access control. By requiring more than one authentication factor, it significantly enhances security compared to traditional single-method locking systems.

A **real-time IoT-based smart door security system** combining:

- Face Recognition (OpenCV + ONNX)
- NFC based Authentication
- Door Sensor Monitoring
- FSM-based Alarm System
- Multi-threaded Event-Driven Architecture

---

#  System Overview

## Smart Door Lock
- Real-time face recognition using camera
- Relay-controlled solenoid lock
- Unlock cooldown + timer-based relocking
- Uses GStreamer (`libcamerasrc`) pipeline

## Door Alarm System
- FSM-based control logic
- Event-driven architecture
- NFC / App authorization
- Alarm trigger on unauthorized access

---

#  System Architecture

## High-Level Architecture

```mermaid
flowchart LR
    CAM[Camera Thread] --> REC[Recognition Thread]
    REC --> BUS[EventBus]

    BUS --> FSM[DoorAlarmFSM]
    FSM --> DOOR[Door Controller]
    FSM --> ALARM[Alarm Manager]

    FSM --> LOGGER[Async Logger]
    LOGGER --> GUI[GUI Server]
```

---

## Event Flow

```mermaid
flowchart TD
    A[Camera Frame] --> B[Face Detection]
    B --> C[Embedding Model]
    C --> D[Compare Database]
    D --> E{Match?}

    E -->|Yes| F[GRANTED Event]
    E -->|No| G[DENIED Event]

    F --> H[FSM]
    G --> H

    H --> I[Door / Alarm Action]
```

---

## Multi-Threaded Design

```mermaid
flowchart TD
    subgraph Threads
        T1[Camera Thread]
        T2[Recognition Thread]
        T3[Control Thread]
        T4[Timer Thread]
        T5[Alarm Thread]
        T6[Logger Thread]
        T7[GUI Thread]
    end

    T1 --> T2
    T2 --> T3
    T3 --> T4
    T3 --> T5
    T3 --> T6
    T6 --> T7
```

---

## Door Control Flow

```mermaid
flowchart TD
    A[Access Event] --> B{Authorized?}
    B -->|No| C[Ignore]
    B -->|Yes| D[Check Cooldown]

    D -->|OK| E[Unlock Relay]
    E --> F[Start Timer]
    F --> G[Relock Door]
```

---

## FSM State Machine

```mermaid
stateDiagram-v2
    [*] --> Disarmed

    Disarmed --> ArmedIdle : Arm
    ArmedIdle --> Disarmed : Disarm

    ArmedIdle --> PendingVerification : Door Open
    PendingVerification --> AuthorizedEntry : NFC/App OK
    PendingVerification --> AlarmActive : Timeout

    AuthorizedEntry --> ArmedIdle : Door Closed
    AlarmActive --> Disarmed : Disarm
```

---

## Event Queue

```mermaid
flowchart LR
    Producers --> Queue[EventQueue] --> FSM

    subgraph Producers
        Camera
        GUI
        Timer
    end
```

---

#  Hardware

| Component            | Notes                                      |
|----------------------|--------------------------------------------|
| Raspberry Pi 5       | 4 GB+ recommended                          |
| Pi Camera Module 3   | Connected via CSI ribbon cable             |
| 5 V relay module     | Active-HIGH; controls 12 V solenoid lock   |
| 12 V solenoid lock   | Fail-secure (locked when unpowered)        |
| 5 V / 3 A PSU        | For the Pi; relay needs its own 12 V rail  |


---

#  Wiring
```
Pi BCM17 ─── Relay IN
Pi 5V   ─── Relay VCC
Pi GND  ─── Relay GND

Relay COM ─── 12V +
Relay NO  ─── Solenoid +
```

---

#  Software Dependencies

```bash
sudo apt update
sudo apt install -y     cmake build-essential git     libgstreamer1.0-dev     gstreamer1.0-plugins-base     gstreamer1.0-plugins-good     gstreamer1.0-plugins-bad     gstreamer1.0-libcamera     libcamera-dev     libgpiod-dev
```

---

#  Project Structure

```
Project Root/
├── main.cpp
├── CMakeLists.txt
├── include/
│   ├── AccessEvent.h
│   ├── AsyncLogger.h
│   ├── CameraThread.h
│   ├── DoorController.h
│   ├── EventBus.h
│   ├── FaceRecognizer.h
│   ├── GpioPin.h
│   ├── OverrideManager.h
│   ├── RecognitionThread.h
│   ├── ThreadSafeQueue.h
│   └── FrameData.h
├── src/
│   ├── AsyncLogger.cpp
│   ├── CameraThread.cpp
│   ├── DoorController.cpp
│   ├── FaceRecognizer.cpp
│   ├── GUIServer.cpp
│   ├── RecognitionThread.cpp
│   └── DoorAlarmSystem.cpp
├── tools/
│   ├── capture_dataset.cpp
│   └── build_database.cpp
├── models/
├── dataset/
└── database/
```

---

#  Workflow

## Capture Dataset
```
./CaptureDataset
```

## Build Database
```
./BuildDatabase
```

## Run System
```
./faceid_door
```

---

#  Alarm Commands

- arm
- disarm
- door_open
- door_close
- nfc_ok
- app_ok
- status
- quit

---

#  Tuning

- kMatchThreshold → accuracy
- kUnlockDurationMs → unlock time
- kCooldownMs → delay
- kDetectEvery → frame skipping

---

# Security Notes

- Not spoof-proof
- Add liveness detection
- Use non-root GPIO access

---

```