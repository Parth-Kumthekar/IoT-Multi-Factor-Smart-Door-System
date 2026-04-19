cmake_minimum_required(VERSION 3.16)
project(SmartDoorSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Dependencies
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# Add all source files from your integrated branch
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
)

# Include directories
target_include_directories(door_alarm PRIVATE 
    include 
    vendor
)

# Link Libraries
# IMPORTANT: 'gpiodcxx' is required for your GPIOPin.cpp code on Pi 5
target_link_libraries(door_alarm PRIVATE
    Threads::Threads
    CURL::libcurl
    gpiodcxx
)

# Compiler Warnings
if(NOT MSVC)
    target_compile_options(door_alarm PRIVATE -Wall -Wextra)
endif()