# IoT-Multi-Factor-Smart-Door-System
An IoT-based multi-factor smart door system that combines NFC authentication, real-time facial recognition, and magnetic door sensors to ensure secure and intelligent access management.

iot-smart-door-unlock/
│
├── docs/                     # Project documentation
│   ├── architecture.md       # System architecture overview
│
├── config/                   # Configuration files
│   ├── default_config.h      # Default firmware configuration
│   └── board_config.h        # Board-specific pin definitions
│
├── include/                  # Header files (.h / .hpp)
│   ├── door_lock.h
│   ├── wifi_manager.h
│   ├── mqtt_client.h
│   ├── auth_manager.h
│   ├── sensor_manager.h
│   └── utils.h
│
├── src/                      # Source files (.cpp)
│   ├── main.cpp              # Entry point
│   ├── 
│   ├── 
│   ├── 
│   ├── 
│   ├── 
│   └── gpio_cntrl.cpp
│
├── hardware/                 # Hardware-related resources
│   └── datasheets/           # Component datasheets
│
├── tests/                    # Unit & integration tests
│   ├── test_door_lock.cpp
│   ├── test_auth.cpp
│   ├── test_wifi.cpp
│   └── mock_components/      # Mock drivers for testing
│
├── scripts/                  # Utility scripts
│   ├── flash_firmware.sh     # Flash firmware to board
│   ├── build.sh              # Build automation script
│   ├── format_code.sh        # clang-format script
│   └── ci_pipeline.yml       # CI configuration (GitHub Actions)
│
├── .gitignore
├── CMakeLists.txt            # CMake build configuration
├── platformio.ini            # (If using PlatformIO)
├── LICENSE
└── README.md
