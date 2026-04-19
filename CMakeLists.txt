cmake_minimum_required(VERSION 3.16)

project(DoorAlarmThreadDesign VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(door_alarm
    src/main.cpp
    src/DoorAlarmSystem.cpp
    src/DoorAlarmFSM.cpp
)

target_include_directories(door_alarm PRIVATE include)