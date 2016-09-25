#-------------------------------------------------
#
# Project created by QtCreator 2016-09-19T14:33:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MultiMonitorConfiguration
TEMPLATE = app

# c++11
CONFIG += c++11

# magick++
QMAKE_CXXFLAGS += $(shell Magick++-config --cppflags --cxxflags)
LIBS += $(shell Magick++-config --ldflags --libs)
INCLUDEPATH += /usr/include/ImageMagick-6/


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    screenconfiglayout.h

FORMS    += mainwindow.ui
