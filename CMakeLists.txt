cmake_minimum_required(VERSION 3.16)
project(SmartDoorSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Dependencies
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# 2. Source Files
# Added EventQueue and AccessController if they are separate .cpp files
add_executable(door_alarm
    src/main.cpp
    src/DoorAlarmSystem.cpp
    src/DoorAlarmFSM.cpp
    src/AlarmManager.cpp
    src/AsyncLogger.cpp
    src/NFCReader.cpp
    src/OutputController.cpp
    src/GPIOPin.cpp
    src/EmailAlert.cpp
    # src/EventQueue.cpp      # Add these if they have .cpp implementations
  src/AccessController.cpp 
)

# 3. Include Directories
# Added 'vendor' explicitly for httplib.h if it's stored there
target_include_directories(door_alarm PRIVATE 
    include 
    vendor
)

# 4. Link Libraries
# 'gpiodcxx' is essential for Pi 5 gpiod character device access
target_link_libraries(door_alarm PRIVATE
    Threads::Threads
    CURL::libcurl
    gpiodcxx
)

# 5. Compiler Optimization & Warnings
if(NOT MSVC)
    target_compile_options(door_alarm PRIVATE 
        -Wall 
        -Wextra 
        -O2      # Added optimization for production
        -pthread # Explicit pthread flag for some GCC versions
    )
endif()

# 6. Installation (Optional but helpful for Pi)
install(TARGETS door_alarm DESTINATION bin)
