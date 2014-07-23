include("includes.cmake")

set(HTTP_Parser_SOURCES http-parser/http_parser.c)
set(GNTP_Send_SOURCES gntp-send/src/growl.c gntp-send/src/md5.c gntp-send/src/tcp.c)

add_library(KCSupport ${HTTP_Parser_SOURCES} ${GNTP_Send_SOURCES})

if(WIN32)
	target_link_libraries(KCSupport ws2_32)
endif(WIN32)
