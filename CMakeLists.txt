cmake_minimum_required(VERSION 3.10)
project(SmartDoor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

include_directories(${GPIOD_INCLUDE_DIRS} include)
link_directories(${GPIOD_LIBRARY_DIRS})

# add_executable(smart_door
#     src/main.cpp
#     src/GPIOPin.cpp
#     src/AccessController.cpp
#     src/OutputController.cpp
#     src/NFCReader.cpp
#     src/DoorController.cpp
# )

add_executable(smart_door
    src/main.cpp
    src/NFCReader.cpp
    src/OutputController.cpp
    src/DoorController.cpp
    src/GPIOPin.cpp
)

target_link_libraries(smart_door ${GPIOD_LIBRARIES})