QT += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

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

OTHER_FILES += ../Info.plist \
	../KanColleTool.rc

target.path = $$PREFIX/bin
INSTALLS += target


# qhttpserver
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/qhttpserver/lib/release/ -lqhttpserver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/qhttpserver/lib/debug/ -lqhttpserver
else:unix: LIBS += -L$$OUT_PWD/../lib/qhttpserver/lib/ -lqhttpserver

INCLUDEPATH += $$PWD/../lib/qhttpserver/src
DEPENDPATH += $$PWD/../lib/qhttpserver/src
