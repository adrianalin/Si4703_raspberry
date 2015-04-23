#-------------------------------------------------
#
# Project created by QtCreator 2015-04-19T19:33:46
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client_radio
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settings.cpp \
    udpclient.cpp

HEADERS  += mainwindow.h \
    settings.h \
    commands.h \
    udpclient.h

FORMS    += mainwindow.ui
