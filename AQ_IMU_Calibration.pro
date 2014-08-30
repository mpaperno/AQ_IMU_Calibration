#-------------------------------------------------
#
# Project created by QtCreator 2014-08-30T10:04:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AQ_IMU_Calibration
TEMPLATE = app

RC_FILE = icon.rc
ICON = resources/app_icon.icns

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h \
    config.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc
