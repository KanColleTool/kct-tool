include("includes.cmake")

add_subdirectory(ehttp)

set(GNTP_Send_SOURCES gntp-send/src/growl.c gntp-send/src/md5.c gntp-send/src/tcp.c)

add_library(KCSupport ${GNTP_Send_SOURCES})
target_link_libraries(KCSupport ehttp)

if(WIN32)
	target_link_libraries(KCSupport ws2_32)
endif(WIN32)
