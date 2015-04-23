#-------------------------------------------------
#
# Project created by QtCreator 2015-04-17T18:34:05
#
#-------------------------------------------------

QT       += core testlib network
CONFIG += c++11
QT       -= gui

TARGET = fm_radio
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    I2Cdev.cpp \
    fmreceiver.cpp \
    server.cpp \
    alarm.cpp

HEADERS += \
    I2Cdev.h \
    fmreceiver.h \
    server.h \
    alarm.h \
    commands.h
