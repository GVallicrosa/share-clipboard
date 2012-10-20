#-------------------------------------------------
#
# Project created by QtCreator 2012-10-12T21:18:47
#
#-------------------------------------------------

QT       += declarative network
CONFIG  += qxt
QXT += core gui


TARGET = shareClipboard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    messagetransceiver.cpp \
    basemessage.cpp \
    imagemessage.cpp \
    filemessage.cpp \
    protocolhandler.cpp \
    clipboardclient.cpp \
    custommessage.cpp \
    notificationwindow.cpp

HEADERS  += mainwindow.h \
    messagetransceiver.h \
    basemessage.h \
    imagemessage.h \
    filemessage.h \
    globals.h \
    protocolhandler.h \
    clipboardclient.h \
    custommessage.h \
    notificationwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    NotificationQml.qml


