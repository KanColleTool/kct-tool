cmake_minimum_required(VERSION 2.8)
project(KanColleTool)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME KanColleToolTest COMMAND KanColleToolTest)

# Add a custom 'make check' target to get output when a test fails
# Taken from http://stackoverflow.com/questions/16158091/
if (CMAKE_CONFIGURATION_TYPES)
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
		--force-new-ctest-process --output-on-failure 
		--build-config "$<CONFIGURATION>")
else()
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
		--force-new-ctest-process --output-on-failure)
endif()
