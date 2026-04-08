cmake_minimum_required(VERSION 3.10)
project(SmartDoor)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(smart_door
    src/main.cpp
    src/gpioevent.cpp
    src/NFCReader.cpp
    src/OutputController.cpp
    src/AccessController.cpp
)

target_link_libraries(smart_door gpiod)