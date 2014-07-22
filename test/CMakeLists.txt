set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif(NOT WIN32)

include_directories("${PROJECT_SOURCE_DIR}/bin")

find_package(Qt5Test)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Network)

if(WIN32)
	find_package(Qt5WinExtras)
endif(WIN32)

add_executable(KanColleToolTest KanColleToolTest.cpp KanColleToolTest.h)
target_link_libraries(KanColleToolTest KCBase Qt5::Test)
