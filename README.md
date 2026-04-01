# FaceID Door Lock — Raspberry Pi 5

A real-time face-recognition door lock built with OpenCV, a Haar cascade
detector, a DNN embedding model, and libgpiod GPIO control.

---

## Hardware

| Component            | Notes                                      |
|----------------------|--------------------------------------------|
| Raspberry Pi 5       | 4 GB+ recommended                          |
| Pi Camera Module 3   | Connected via CSI ribbon cable             |
| 5 V relay module     | Active-HIGH; controls 12 V solenoid lock   |
| 12 V solenoid lock   | Fail-secure (locked when unpowered)        |
| 5 V / 3 A PSU        | For the Pi; relay needs its own 12 V rail  |

### Wiring

```
Pi 5 BCM 17 (pin 11) ──── Relay IN
Pi 5 5 V     (pin 2) ──── Relay VCC
Pi 5 GND     (pin 6) ──── Relay GND
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
    libcamera-v4l2          # exposes Pi Camera through V4L2
```

Add to `/boot/firmware/config.txt` (Camera Module 3):
```
dtoverlay=imx708
```
Or for Camera Module 2:
```
dtoverlay=imx219
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
├── capture_dataset.cpp
├── build_database.cpp
├── models/
│   ├── haarcascade_frontalface_default.xml   ← from OpenCV data/
│   └── face_recognition.onnx                ← see below
├── dataset/            ← created by capture_dataset
└── database/           ← created by build_database
    └── embeddings.yml
```

### Getting the ONNX model

A good open-source option is **ArcFace ResNet-50** from the InsightFace project:

```bash
# Install the Python helper (on the Pi or a PC)
pip install insightface onnxruntime
python - <<'EOF'
import insightface, os
app = insightface.app.FaceAnalysis(providers=["CPUExecutionProvider"])
app.prepare(ctx_id=0)
# The .onnx files are cached in ~/.insightface/models/
EOF
# Copy the recognition model
cp ~/.insightface/models/buffalo_l/w600k_r50.onnx models/face_recognition.onnx
```

The Haar cascade ships with OpenCV:
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

### 1. Capture dataset images

```bash
./build/capture_dataset
# Enter a name, press 'c' ~10–20 times, then 'q'
# Repeat for each person
```

### 2. Build the embeddings database

```bash
./build/build_database
# Produces database/embeddings.yml
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

| Constant            | Location    | Effect                                   |
|---------------------|-------------|------------------------------------------|
| `kMatchThreshold`   | main.cpp    | Higher = stricter (reduce false accepts) |
| `kUnlockDurationMs` | main.cpp    | How long the door stays open             |
| `kCooldownMs`       | main.cpp    | Min time between unlock events           |
| `kDetectEvery`      | FaceRecognizer.h | Skip N frames between detections   |

---

## Security notes

- Cosine similarity on a single embedding is adequate for a home project but
  is **not** hardened against spoofing (printed photo, video replay).
- For higher security add liveness detection (blink / depth camera).
- Run as a dedicated non-root user; grant GPIO access via the `gpio` group
  (`sudo usermod -aG gpio $USER`).
