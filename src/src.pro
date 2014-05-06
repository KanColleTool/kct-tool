QT += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

windows: QT += winextras

TARGET = KanColleTool
unix:!macx: TARGET = kancolletool
TEMPLATE = app

include(../KanColleTool.pri)

QMAKE_INFO_PLIST = ../Info.plist
RC_FILE = ../KanColleTool.rc
ICON = ../resources/KanColleTool.icns

macx: LIBS += -framework Carbon -lobjc

FORMS += \
	../forms/KCMainWindow.ui \
	../forms/KCSettingsDialog.ui

RESOURCES += ../resources/resources.qrc

SOURCES += 	main.cpp

TRANSLATIONS += \
	../translations/KanColleTool_en_UK.ts

target.path = $$PREFIX/bin
INSTALLS += target

OTHER_FILES += ../Info.plist \
	../KanColleTool.rc
