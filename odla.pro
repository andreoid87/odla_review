#-------------------------------------------------
#
# Project created by QtCreator 2018-10-08T12:06:58
#
#-------------------------------------------------

# put product name in a define
DEFINES += PRODUCT_NAME='\\"'ODLA'\\"'

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

# put version string in a define
VERSION = $$system(git describe --tag --abbrev=0)
DEFINES += VERSION='\\"$${VERSION}\\"'
message(Version detected by git: $$VERSION)

QMAKE_TARGET_COMPANY = Kemonia River s.r.l.
QMAKE_TARGET_COPYRIGHT = Kemonia River s.r.l.


QT += core gui widgets network texttospeech sql websockets serialport
TARGET = Odla
TEMPLATE = app

target.path = $$OUT_PWD/Odla
INSTALLS += target

SOURCES += \
    src/app.cpp \
    src/appcustom.cpp \
    src/appdorico.cpp \
    src/appmusescore3.cpp \
    src/appmusescore4.cpp \
    src/button.cpp \
    src/buttoncommand.cpp \
    src/buttoninsertion.cpp \
    src/buttonnumber.cpp \
    src/buttontoggle.cpp \
    src/database.cpp \
    src/keyboard.cpp \
    src/main.cpp \
    #src/commons.cpp \
    src/menu.cpp \
    src/menuinsertion.cpp \
    src/menustandard.cpp \
    src/menuvcenter.cpp \
    src/metadata.cpp \
    src/panel.cpp \
    src/pthread_barrier.c \
    src/voiceover.cpp

HEADERS += \
    src/app.h \
    src/appcustom.h \
    src/appdorico.h \
    src/appmusescore3.h \
    src/appmusescore4.h \
    src/button.h \
    src/buttoncommand.h \
    src/buttoninsertion.h \
    src/buttonnumber.h \
    src/buttontoggle.h \
    #src/commons.h \
    src/database.h \
    src/keyboard.h \
    src/menu.h \
    src/menuinsertion.h \
    src/menustandard.h \
    src/menuvcenter.h \
    src/metadata.h \
    src/panel.h \
    src/pthread_barrier.h \
    src/hidapi.h \
    src/voiceover.h

RESOURCES += \
    files.qrc

macx {
    HEADERS += src/appnap.h
    OBJECTIVE_SOURCES += src/appnap.m
    SOURCES += src/hid_mac.c
    #INCLUDEPATH += /usr/local/opt/libiconv/include/
    # Trova il percorso degli include di libusb
    LIBUSB_INCLUDE_PATH = $$system("locate libusb-1.0 | grep /include/libusb-1.0 | head -n 1")
    INCLUDEPATH += $$LIBUSB_INCLUDE_PATH
    DEPENDPATH += $$LIBUSB_INCLUDE_PATH
    message("Find include libusb in: $$LIBUSB_INCLUDE_PATH")

    # Trova il percorso della libreria statica di libusb
    LIBUSB_LIB_PATH = $$system("locate libusb-1.0.a | grep /lib/libusb-1.0.a | head -n 1")
    PRE_TARGETDEPS += $$LIBUSB_LIB_PATH
    LIBS += $$LIBUSB_LIB_PATH
    message("Find libreria libusb in: $$LIBUSB_LIB_PATH")


    LIBS += -framework IOKit
    LIBS += -framework CoreFoundation
    LIBS += -framework Foundation
    QMAKE_LFLAGS += -F/Library/Frameworks/
    DISTFILES += appnap.m
    DEFINES += ICON_NAME='\\"'odla.icns'\\"'
    DISTFILES += odla.icns
    ICON = odla.icns
    RC_ICONS = odla.icns
    QMAKE_TARGET_BUNDLE_PREFIX = org.odlamusic.odla
}

win32 {
    SOURCES += src/hid_windows.c
    LIBS += -lsetupapi
    DISTFILES += odla.ico
    DEFINES += ICON_NAME='\\"'odla.ico'\\"'
    ICON = odla.ico
    RC_ICONS = odla.ico
}

unix:!macx {
    SOURCES += src/hid_linux.c
    LIBS += -lusb-1.0 -lrt -lpthread
    INCLUDEPATH += /usr/include/libusb-1.0
}
