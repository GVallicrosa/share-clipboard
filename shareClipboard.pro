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


###########
## Zeroconf
###########

# Dependencies: "libavahi-compat-libdnssd1" package

#
# qmake -config zeroconf
#
zeroconf {
    DEFINES += ZEROCONF
    message("Zeroconf support")

    unix {
        CONFIG += link_pkgconfig
        PKGCONFIG += avahi-compat-libdns_sd
    }

    #!mac:x11:LIBS+=-ldns_sd
    #win32:LIBS+=-ldnssd
    #LIBPATH=C:/Temp/mDNSResponder-107.6/mDNSWindows/DLL/Debug
    #INCLUDEPATH += c:/Temp/mDNSResponder-107.6/mDNSShared


    SOURCES  += avahi/bonjourserviceresolver.cpp \
                avahi/bonjourserviceregister.cpp \
                avahi/bonjourservicebrowser.cpp

    HEADERS +=  avahi/bonjourserviceresolver.h \
                avahi/bonjourserviceregister.h \
                avahi/bonjourservicebrowser.h \
                avahi/bonjourrecord.h

}
