cmake_minimum_required(VERSION 3.16)

project(door_alarm_api_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(door_alarm_api
    src/main.cpp
)

target_include_directories(door_alarm_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

if (MSVC)
    target_compile_options(door_alarm_api PRIVATE /W4)
else()
    target_compile_options(door_alarm_api PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)
target_link_libraries(door_alarm_api PRIVATE Threads::Threads)