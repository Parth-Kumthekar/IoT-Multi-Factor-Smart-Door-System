cmake_minimum_required(VERSION 3.18)
project(SmartDoorSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Libraries FIRST
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
pkg_check_modules(NFC REQUIRED libnfc)

# 2. Tell CMake where your local .hpp files are
include_directories(include)
# Also include the system paths found by PkgConfig
include_directories(${GPIOD_INCLUDE_DIRS} ${NFC_INCLUDE_DIRS})

# 3. Collect source files (Excluding main.cpp if you use it in tests)
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 4. Create the Main Executable
add_executable(smart_door_demo ${SOURCES})

# 5. Link Libraries to Main Executable
target_link_libraries(smart_door_demo PRIVATE 
    ${GPIOD_LIBRARIES} 
    ${NFC_LIBRARIES}
)

# 6. Add the Test Executable
# Note: Ensure test_door.cpp doesn't have its own 'main' if it conflicts with src/main.cpp
add_executable(run_tests 
    test/test_door.cpp 
    src/OutputHandler.cpp  # Added specifically for hardware logic
    src/DoorController.cpp
    src/NFCReader.cpp
)

target_link_libraries(run_tests PRIVATE 
    ${GPIOD_LIBRARIES} 
    ${NFC_LIBRARIES}
)