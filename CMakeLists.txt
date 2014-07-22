cmake_minimum_required(VERSION 3.0)
project(KanColleTool)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME KanColleToolTest COMMAND KanColleToolTest)

if(${CMAKE_GENERATOR} MATCHES ".* Makefiles")
	# Add a custom 'make check' target to get output when a test fails
	# Taken from http://stackoverflow.com/questions/16158091/
	if(CMAKE_CONFIGURATION_TYPES)
		add_custom_target(check_tool COMMAND ${CMAKE_CTEST_COMMAND} 
			--force-new-ctest-process --output-on-failure 
			--build-config "$<CONFIGURATION>")
	else()
		add_custom_target(check_tool COMMAND ${CMAKE_CTEST_COMMAND} 
			--force-new-ctest-process --output-on-failure)
	endif()
endif()
