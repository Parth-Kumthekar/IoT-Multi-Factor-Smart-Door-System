# FaceID Door Lock — Raspberry Pi 5

An IoT-based multi-factor smart door system combining real-time face
recognition, NFC authentication, and magnetic door sensors for secure
and intelligent access management.

---

## Hardware

| Component | Notes |
|---|---|
| Raspberry Pi 5 (4 GB+) | Main compute unit |
| Pi Camera Module 3 | Connected via CSI ribbon |
| 5 V relay module | Active-HIGH; drives solenoid |
| 12 V solenoid lock | Fail-secure (locked when unpowered) |
| 5 V / 3 A PSU | Powers the Pi |

### Wiring

```
Pi 5 BCM 17 (pin 11) ──── Relay IN
Pi 5 5 V    (pin  2) ──── Relay VCC
Pi 5 GND    (pin  6) ──── Relay GND
Relay COM ──────────────── 12 V +
Relay NO  ──────────────── Solenoid +
Solenoid  ──────────────── 12 V GND
```

> **Pi 5 note**: GPIO is exposed via `/dev/gpiochip4` (RP1 controller).
> The code tries `gpiochip4` first and falls back to `gpiochip0` for Pi 4/3.

---

## Software dependencies

```bash
sudo apt update
sudo apt install -y \
    cmake build-essential \
    libopencv-dev \
    libgpiod-dev \
    libcamera-v4l2
```

Add to `/boot/firmware/config.txt`:

```
# Camera Module 3
dtoverlay=imx708

# Camera Module 2 (use instead if needed)
# dtoverlay=imx219
```

---

## Project layout

```
project_root/
├── CMakeLists.txt
├── main.cpp
├── FaceRecognizer.h / .cpp
├── DoorLock.h / .cpp
├── ThreadSafeQueue.h
├── FrameData.h
├── Callback.h
├── scripts/
│   ├── capture_dataset.cpp
│   └── build_database.cpp
├── models/
│   ├── haarcascade_frontalface_default.xml
│   └── face_recognition.onnx
├── dataset/            ← created by capture_dataset
└── database/
    └── embeddings.yml  ← created by build_database
```

---

## Getting the ONNX model

```bash
pip install insightface onnxruntime
python3 - <<'EOF'
import insightface
app = insightface.app.FaceAnalysis(providers=["CPUExecutionProvider"])
app.prepare(ctx_id=0)
EOF
cp ~/.insightface/models/buffalo_l/w600k_r50.onnx models/face_recognition.onnx
```

Copy the Haar cascade from OpenCV:

```bash
cp /usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml models/
```

---

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

---

## Workflow

### 1. Enrol faces

```bash
./build/capture_dataset
# Enter a name → press 'c' 10–20 times to capture → 'q' for next person → type 'exit'
```

### 2. Build embedding database

```bash
./build/build_database
# Writes database/embeddings.yml
```

### 3. Run the door lock

```bash
./build/faceid_door
# Press ESC to quit
```

The door unlocks for **3 seconds** on a confirmed match.  
A **5-second cooldown** prevents repeated triggers.

---

## Tuning

| Constant | File | Effect |
|---|---|---|
| `kMatchThreshold` | `FaceRecognizer.h` | Higher = stricter (fewer false accepts) |
| `kUnlockDurationMs` | `DoorLock.h` | How long door stays open (ms) |
| `kCooldownMs` | `DoorLock.h` | Min time between unlock events (ms) |
| `kDetectEvery` | `FaceRecognizer.h` | Skip N frames between detections |
| `kGpioPin` | `DoorLock.h` | BCM GPIO pin number for relay |

---

## Security notes

- Cosine similarity on a single embedding is suitable for a home project
  but is **not** hardened against spoofing (printed photo, video replay).
- For higher security add liveness detection (blink challenge / depth camera).
- Run as a dedicated non-root user; grant GPIO access via the `gpio` group:
```bash
  sudo usermod -aG gpio $USER
```

