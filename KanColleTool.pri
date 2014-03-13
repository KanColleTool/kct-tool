CONFIG += c++11

# This is updated by the version bump script
VERSION = 0.8.5

INCLUDEPATH += . ..
WARNINGS += -Wall

isEmpty(PREFIX): PREFIX=/usr

HEADERS  += \
	../src/KCMainWindow.h \
	../src/KCSettingsDialog.h \
	../src/KCClient.h \
	../src/KCToolServer.h \
	../src/KCGameObject.h \
	../src/KCShip.h \
	../src/KCShipType.h \
	../src/KCFleet.h \
	../src/KCDock.h \
	../src/KCTranslator.h \
	../src/KCLogger.h \
	../src/KCDefaults.h

SOURCES += \
	../src/KCMainWindow.cpp \
	../src/KCSettingsDialog.cpp \
	../src/KCClient.cpp \
	../src/KCClient_p.cpp \
	../src/KCToolServer.cpp \
	../src/KCShip.cpp \
	../src/KCShipType.cpp \
	../src/KCFleet.cpp \
	../src/KCDock.cpp \
	../src/KCTranslator.cpp \
	../src/KCLogger.cpp

UI_DIR = ../build/uics
RCC_DIR = ../build/rccs
MOC_DIR = ../build/mocs
OBJECTS_DIR = ../build/objs
DESTDIR = ../bin