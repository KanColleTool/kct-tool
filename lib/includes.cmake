if(MSVC)
	# No, gmtime() is not an insecure function, fuck you
	add_definitions(/D_SCL_SECURE_NO_WARNINGS=1)
endif()

include_directories(${PROJECT_SOURCE_DIR}/lib/kclib/src)

include_directories(${PROJECT_SOURCE_DIR}/lib/ehttp/include)

add_definitions(-DGROWL_STATIC)
include_directories(${PROJECT_SOURCE_DIR}/lib/gntp-send/include)
