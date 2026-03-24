cmake_minimum_required(VERSION 3.15)
project(FaceRecognitionSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
set(CMAKE_PREFIX_PATH "C:\\opencv\\build\\x64\\vc16\\lib" ${CMAKE_PREFIX_PATH})
find_package(OpenCV REQUIRED)

# Include headers
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    include 
)


# Main Face Recognition App

add_executable(FaceID
    main.cpp
    src/FaceRecognizer.cpp
)

target_link_libraries(FaceID ${OpenCV_LIBS})


# Dataset Capture Tool

add_executable(CaptureDataset
    tools/capture_dataset.cpp
)

target_link_libraries(CaptureDataset ${OpenCV_LIBS})


# Database Builder Tool

add_executable(BuildDatabase
    tools/build_database.cpp
)

target_link_libraries(BuildDatabase ${OpenCV_LIBS})