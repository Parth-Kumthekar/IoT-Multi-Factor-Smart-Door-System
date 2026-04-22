cmake_minimum_required(VERSION 3.16)
project(SmartDoorSystem VERSION 1.0.0 LANGUAGES CXX)

add_definitions(-DREAL_GPIO)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find System Dependencies
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

# IoT & Camera specific modules
pkg_check_modules(LIBGPIOD REQUIRED libgpiod)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

# 2. Source Definitions
# Core logic shared between main app and tests
set(CORE_SOURCES
    src/DoorAlarmFSM.cpp
    src/AlarmManager.cpp
    src/AsyncLogger.cpp
    src/EmailAlert.cpp
    src/AccessController.cpp
    src/OutputController.cpp
    src/NFCReader.cpp
    src/GPIOPin.cpp
)

# Face Recognition specific sources

set(FACE_ID_SOURCES
    src/LoggerCam.cpp
    src/DoorLock.cpp
    src/FaceRecognizer.cpp
    src/CameraThread.cpp
    src/RecognitionThread.cpp
    #src/GUIServer.cpp
)

# 3. Main Application Executable
# This now includes both the Door logic and the FaceID logic
add_executable(door_alarm
#src/main_1stintegration.cpp
    src/main.cpp
    src/DoorAlarmSystem.cpp
    ${CORE_SOURCES}
    ${FACE_ID_SOURCES}
)

# 4. Tools & Utilities
add_executable(capture_dataset tools/capture_dataset.cpp)
add_executable(build_database tools/build_database.cpp)
add_executable(run_tests tests/tests.cpp ${CORE_SOURCES})

# 5. Include Directories
target_include_directories(door_alarm PRIVATE 
    include 
    vendor 
    ${OpenCV_INCLUDE_DIRS}
    ${LIBGPIOD_INCLUDE_DIRS}
    ${LIBCAMERA_INCLUDE_DIRS}
)

# Ensure utilities can find headers
target_include_directories(capture_dataset PRIVATE include ${OpenCV_INCLUDE_DIRS})
target_include_directories(build_database PRIVATE include ${OpenCV_INCLUDE_DIRS})
target_include_directories(run_tests PRIVATE include vendor)

# 6. Link Libraries
# The main app needs EVERYTHING
target_link_libraries(door_alarm PRIVATE
    Threads::Threads
    CURL::libcurl
    gpiodcxx
    ${OpenCV_LIBS}
    ${LIBGPIOD_LIBRARIES}
    lccv
    pthread
)

# Link tools with minimal necessary libs
target_link_libraries(capture_dataset PRIVATE ${OpenCV_LIBS} pthread lccv)
target_link_libraries(build_database PRIVATE ${OpenCV_LIBS} pthread)
target_link_libraries(run_tests PRIVATE Threads::Threads CURL::libcurl gpiodcxx)
