include("includes.cmake")

add_subdirectory(kclib)
find_package(Qt5Network)	# Needed for it to link to kclib properly, for some reason...

add_subdirectory(ehttp)
set_property(TARGET ehttp PROPERTY FOLDER "3rd Party")

set(GNTP_Send_SOURCES gntp-send/src/growl.c gntp-send/src/md5.c gntp-send/src/tcp.c)

add_library(KCSupport ${GNTP_Send_SOURCES})
target_link_libraries(KCSupport kclib ehttp)

if(WIN32)
	target_link_libraries(KCSupport ws2_32)
endif(WIN32)
