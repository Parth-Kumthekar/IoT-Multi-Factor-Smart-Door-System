cmake_minimum_required(VERSION 3.20)
project(API LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(API
    src/main.cpp
    src/system_state.cpp
    src/api_server.cpp
)

target_include_directories(API PRIVATE
    src
    external
)
