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



#
# --> Find libraries and stuff
#
include_directories("${PROJECT_SOURCE_DIR}/lib/http-parser")

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Network)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
)
add_definitions(
	${Qt5Core_DEFINITIONS}
	${Qt5Widgets_DEFINITIONS}
	${Qt5Network_DEFINITIONS}
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Network_EXECUTABLE_COMPILE_FLAGS}")

if(APPLE)
	find_library(LIBOBJC objc)
	find_library(CARBON Carbon)
	list(APPEND KanColleTool_EXTRALIBS ${LIBOBJC} ${CARBON})
endif(APPLE)



#
# --> Find sources
#
file(GLOB_RECURSE KanColleTool_FORMS ../forms/*.ui)
file(GLOB_RECURSE KanColleTool_SOURCES *.cpp *.h)
file(GLOB KanColleTool_MAIN_CPP main.cpp)
list(REMOVE_ITEM KanColleTool_SOURCES ${KanColleTool_MAIN_CPP})

# Remove moc and uic files from the source list
file(GLOB KanColleTool_MOCS moc_*.cpp *_automoc.cpp ui_*.h)
list(REMOVE_ITEM KanColleTool_SOURCES "" ${KanColleTool_MOCS})

qt5_wrap_ui(KanColleTool_UIHEADERS ${KanColleTool_FORMS})




#
# --> Build the common library
#
add_library(KCBase
	${KanColleTool_SOURCES}
	${KanColleTool_UIHEADERS}
	${PROJECT_SOURCE_DIR}/lib/http-parser/http_parser.c
)
target_link_libraries(KCBase
	Qt5::Core Qt5::Widgets Qt5::Network
	${KanColleTool_EXTRALIBS}
)



#
# --> Build the main application
#
add_executable(KanColleTool WIN32 MACOSX_BUNDLE ${KanColleTool_MAIN_CPP})
target_link_libraries(KanColleTool KCBase)
