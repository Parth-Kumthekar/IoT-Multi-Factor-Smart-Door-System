# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(smart_door)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    include
    /home/vishwas_maney/libgpiod_build/include
)

link_directories(
    /home/vishwas_maney/libgpiod_build/lib
)

add_executable(smart_door
    src/main.cpp
    src/NFCReader.cpp
    src/OutputController.cpp
    src/DoorController.cpp
)

target_link_libraries(smart_door gpiod)