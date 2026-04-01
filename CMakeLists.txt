cmake_minimum_required(VERSION 3.18)
project(SmartDoorSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Libraries

# Link the library to your executable
target_link_libraries(smart_door_demo PRIVATE ${GPIOD_LIBRARIES})
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
pkg_check_modules(NFC REQUIRED libnfc)

# 1. Tell CMake where the .hpp files are
include_directories(include)

# 2. Collect all .cpp files from the src folder
file(GLOB SOURCES "src/*.cpp")

# 3. Create the executable
add_executable(smart_door_demo ${SOURCES})

# 4. Link everything
target_link_libraries(smart_door_demo PRIVATE 
    ${GPIOD_LIBRARIES} 
    ${NFC_LIBRARIES}

#5. Add the test executable
add_executable(run_tests 
    test/test_door.cpp 
    src/DoorController.cpp 
    # Add other .cpp files needed for testing
)

target_link_libraries(run_tests PRIVATE 
    ${GPIOD_LIBRARIES} 
    ${NFC_LIBRARIES}
)

)