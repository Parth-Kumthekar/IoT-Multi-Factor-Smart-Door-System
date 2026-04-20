# IoT-based Mutli-Factor Door Lock system — Raspberry Pi 5

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


---

## Software dependencies

```bash
sudo apt update
sudo apt install -y \
    cmake build-essential git \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libcamera \
    gstreamer1.0-tools \
    gstreamer1.0-libav \
    libcamera-dev \
    libavcodec-dev libavformat-dev libswscale-dev \
    libgtk-3-dev libjpeg-dev libpng-dev libtiff-dev \
    libgpiod-dev

## Clone OpenCV
cd ~
git clone --depth 1 --branch 4.10.0 https://github.com/opencv/opencv.git
git clone --depth 1 --branch 4.10.0 https://github.com/opencv/opencv_contrib.git

# Build with GStreamer ON
mkdir opencv/build && cd opencv/build
cmake .. \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DOPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
    -DWITH_GSTREAMER=ON \
    -DWITH_V4L=ON \
    -DWITH_LIBV4L=ON \
    -DENABLE_NEON=ON \
    -DWITH_OPENMP=ON \
    -DBUILD_TESTS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DINSTALL_C_EXAMPLES=OFF \
    -DOPENCV_GENERATE_PKGCONFIG=ON

make -j4
sudo make install
sudo ldconfig

# Verify GStreamer is now YES
python3 -c "import cv2; info=cv2.getBuildInformation(); \
    print([l for l in info.splitlines() if 'GStreamer' in l])"
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
Project Root/
│
├── main.cpp                       ← owns all objects, signal handler
├── CMakeLists.txt                 ← platform-aware build
│
├── include/
│   ├── AccessEvent.h              ← shared event type
│   ├── AsyncLogger.h              ← non-blocking background logger
│   ├── CameraThread.h             ← camera capture thread
│   ├── DoorController.h           ← GPIO + CV timer
│   ├── EventBus.h                 ← publish/subscribe hub
│   ├── FaceRecognizer.h           ← DNN face recognition
│   ├── GpioPin.h                  ← gpiod wrapper
│   ├── OverrideManager.h          ← bypass toggle
│   ├── RecognitionThread.h        ← pulls frames, publishes events
│   ├── ThreadSafeQueue.h          ← CV-based queue, optional pop
│   ├── FrameData.h                ← timestamped frame struct
│   └── gpiod_mock.h               ← Windows/Mac stub for PC references 
│
├── src/
│   ├── AsyncLogger.cpp
│   ├── CameraThread.cpp              
│   ├── DoorController.cpp
│   ├── FaceRecognizer.cpp
│   ├── GUIServer.h + GUIServer.cpp
│   ├── RecognitionThread.cpp
│   └── SignalHandler.h
│
├── scripts/
│   ├── capture_dataset.cpp
│   └── build_database.cpp
│
├── models/
│   ├── face_recognition.onnx        
│   └── haarcascade_frontalface_default.xml
│
├── database/         ← created at runtime
├── dataset/          ← created by capture_dataset

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

