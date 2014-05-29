# 
# Note: This builds the project as an executable linked to a library built from
# its sources. This would be stupid and roundabout if it wasn't for how it also
# lets us share code between application and unit tests with no extra effort.
# 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
include_directories("${PROJECT_SOURCE_DIR}/lib/http-parser")

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Network)

file(GLOB_RECURSE KanColleTool_FORMS ../forms/*.ui)
file(GLOB_RECURSE KanColleTool_SOURCES *.cpp *.h)
file(GLOB KanColleTool_MAIN_CPP main.cpp)
list(REMOVE_ITEM KanColleTool_SOURCES ${KanColleTool_MAIN_CPP})

# Remove moc_* and *_automoc.cpp files from the source list
file(GLOB_RECURSE KanColleTool_MOCS moc_*.cpp *_automoc.cpp)
list(REMOVE_ITEM KanColleTool_SOURCES "${KanColleTool_MOCS}")

if(APPLE)
	find_library(LIBOBJC objc)
	find_library(CARBON Carbon)
	set(KanColleTool_EXTRALIBS ${LIBOBJC} ${CARBON})
endif(APPLE)

qt5_wrap_ui(KanColleTool_UIHEADERS ${KanColleTool_FORMS})

add_library(KCBase
	${KanColleTool_SOURCES}
	${KanColleTool_UIHEADERS}
	${PROJECT_SOURCE_DIR}/lib/http-parser/http_parser.c
)
target_link_libraries(KCBase
	Qt5::Core Qt5::Widgets Qt5::Network
	${KanColleTool_EXTRALIBS}
)

add_executable(KanColleTool WIN32 MACOSX_BUNDLE ${KanColleTool_MAIN_CPP})
target_link_libraries(KanColleTool KCBase)
