cmake_minimum_required(VERSION 3.16)
project(SmartDoorSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find System Dependencies
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# 2. Comprehensive Source Definitions
# CORE_SOURCES: Logic that can be tested WITHOUT Raspberry Pi hardware.
set(CORE_SOURCES
    src/DoorAlarmFSM.cpp
    src/AlarmManager.cpp
    src/AsyncLogger.cpp
    src/EmailAlert.cpp
    src/AccessController.cpp
    src/EventQueue.cpp 
)

# HARDWARE_SOURCES: Logic that requires libgpiod or serial drivers (Pi specific).
set(HARDWARE_SOURCES
    src/NFCReader.cpp
    src/OutputController.cpp
    src/GPIOPin.cpp
    src/DoorAlarmSystem.cpp
)

# 3. Main Application Executable
# Combines everything + main.cpp
add_executable(door_alarm
    src/main.cpp
    ${CORE_SOURCES}
    ${HARDWARE_SOURCES}
)

# 4. Test Executable
# Only includes logic files + tests.cpp (Allows testing on a PC/Laptop)
add_executable(run_tests 
    tests/tests.cpp 
    ${CORE_SOURCES}
)

# 5. Include Directories
target_include_directories(door_alarm PRIVATE include vendor)
target_include_directories(run_tests PRIVATE include vendor)

# 6. Link Libraries
target_link_libraries(door_alarm PRIVATE
    Threads::Threads
    CURL::libcurl
    gpiodcxx
)

target_link_libraries(run_tests PRIVATE
    Threads::Threads
    CURL::libcurl
)

# 7. Build Settings
if(NOT MSVC)
    set(COMPILE_FLAGS -Wall -Wextra -Wpedantic -O2 -pthread)
    target_compile_options(door_alarm PRIVATE ${COMPILE_FLAGS})
    target_compile_options(run_tests PRIVATE ${COMPILE_FLAGS})
endif()

# 8. CTest Registration
enable_testing()
add_test(NAME LogicTests COMMAND run_tests)