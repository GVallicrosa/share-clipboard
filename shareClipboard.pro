#-------------------------------------------------
#
# Project created by QtCreator 2012-10-12T21:18:47
#
#-------------------------------------------------

QT       += network
CONFIG  += qxt
QXT += core gui


TARGET = shareClipboard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    messagetransceiver.cpp

HEADERS  += mainwindow.h \
    messagetransceiver.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc


