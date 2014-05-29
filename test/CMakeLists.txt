cmake_minimum_required(VERSION 2.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -fPIE")
include_directories("${PROJECT_SOURCE_DIR}/build")

find_package(Qt5Core)
find_package(Qt5Test)
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Test_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Test_DEFINITIONS})

add_executable(KanColleToolTest KanColleToolTest.cpp KanColleToolTest.h)
target_link_libraries(KanColleToolTest KCBase Qt5::Test)
