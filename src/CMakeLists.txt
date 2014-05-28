set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
include_directories("${PROJECT_SOURCE_DIR}/lib/http-parser")

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Network)

file(GLOB_RECURSE KanColleTool_FORMS ../forms/*.ui)
file(GLOB_RECURSE KanColleTool_SOURCES *.cpp *.h)

if(APPLE)
	find_library(LIBOBJC objc)
	find_library(CARBON Carbon)
	set(KanColleTool_EXTRALIBS ${LIBOBJC} ${CARBON})
endif(APPLE)

qt5_wrap_ui(KanColleTool_UIHEADERS ${KanColleTool_FORMS})

add_executable(KanColleTool
	WIN32 MACOSX_BUNDLE
	${KanColleTool_SOURCES}
	${KanColleTool_UIHEADERS}
	${PROJECT_SOURCE_DIR}/lib/http-parser/http_parser.c
)
target_link_libraries(KanColleTool
	Qt5::Core Qt5::Widgets Qt5::Network
	${KanColleTool_EXTRALIBS}
)
