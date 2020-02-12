######################################################################
# Automatically generated by qmake (3.1) Fri Oct 11 17:35:00 2019
######################################################################

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = ukui-volume-control-applet-qt
INCLUDEPATH += .

include(QtSingleApplication/qtsingleapplication.pri)

DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
target.path = /usr/bin

#inst.path += /usr/share/ukui-media/translations
#inst.files += translations/ukui-media-volume-control-applet-qt_zh_CN.qm
#              translations/1

INSTALLS += \
    target

CONFIG += \
    c++11 \
    debug \
    no_keywords \
    link_pkgconfig \

PKGCONFIG += \
    gtk+-3.0 \
    glib-2.0 \
    libmatemixer \
    gsettings-qt \
    Qt5Svg  \
    gio-2.0 \
    Qt5Xdg

HEADERS += \
    ukmedia_device_switch_widget.h \
    ukmedia_device_volume_widget.h \
    ukmedia_scroll_widget.h \
    ukmedia_application_volume_widget.h\


SOURCES += \
    main.cpp \
    ukmedia_device_switch_widget.cpp \
    ukmedia_device_volume_widget.cpp\
    ukmedia_application_volume_widget.cpp \
    ukmedia_scroll_widget.cpp

RESOURCES += \
    res.qrc

TRANSLATIONS += \
    translations/ukui-volume-control-applet-qt-zh_CN.ts
