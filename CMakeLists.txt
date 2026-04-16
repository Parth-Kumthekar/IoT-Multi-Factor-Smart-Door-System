cmake_minimum_required(VERSION 3.16)
project(FaceIDDoorLock CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOD REQUIRED libgpiod)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${LIBGPIOD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
)

# Main door-lock binary 
add_executable(faceid_door
    main.cpp
    FaceRecognizer.cpp
    DoorLock.cpp
)
target_link_libraries(faceid_door
    ${OpenCV_LIBS}
    ${LIBGPIOD_LIBRARIES}
    pthread
)

# Dataset capture utility 
add_executable(capture_dataset
    scripts/capture_dataset.cpp
)
target_link_libraries(capture_dataset
    ${OpenCV_LIBS}
    pthread
)

#Embedding database builder 
add_executable(build_database
    scripts/build_database.cpp
)
target_link_libraries(build_database
    ${OpenCV_LIBS}
    pthread
)