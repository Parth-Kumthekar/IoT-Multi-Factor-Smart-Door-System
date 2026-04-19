cmake_minimum_required(VERSION 3.10)
project(smart_door)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(smart_door
    src/main.cpp
    src/GPIOPin.cpp
    src/OutputController.cpp
    src/NFCReader.cpp
    src/AccessController.cpp
)

target_link_libraries(smart_door gpiod gpiodcxx)