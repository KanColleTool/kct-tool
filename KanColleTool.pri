CONFIG += c++11

# This is updated by the version bump script
VERSION = 0.9.0

INCLUDEPATH += . ..
WARNINGS += -Wall

isEmpty(PREFIX): PREFIX=/usr

HEADERS  += \
	../src/KCMainWindow.h \
	../src/KCSettingsDialog.h \
	../src/KCClient.h \
	../src/KCToolServer.h \
	../src/KCToolServerResponder.h \
	../src/KCGameObject.h \
	../src/KCShip.h \
	../src/KCShipType.h \
	../src/KCFleet.h \
	../src/KCDock.h \
	../src/KCAdmiral.h \
	../src/KCTranslator.h \
	../src/KCDefaults.h \
	../src/KCUtil.h

SOURCES += \
	../src/KCMainWindow.cpp \
	../src/KCSettingsDialog.cpp \
	../src/KCClient.cpp \
	../src/KCClient_p.cpp \
	../src/KCToolServer.cpp \
	../src/KCToolServerResponder.cpp \
	../src/KCShip.cpp \
	../src/KCShipType.cpp \
	../src/KCFleet.cpp \
	../src/KCDock.cpp \
	../src/KCAdmiral.cpp \
	../src/KCTranslator.cpp \
	../lib/http-parser/http_parser.c

UI_DIR = ../build/uics
RCC_DIR = ../build/rccs
MOC_DIR = ../build/mocs
OBJECTS_DIR = ../build/objs
DESTDIR = ../bin

INCLUDEPATH += ../lib/http-parser
#LIBS += -L../lib/http-parser -lhttp_parser
