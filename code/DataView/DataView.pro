# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------

TEMPLATE = lib
TARGET = DataView
DESTDIR = ../x86/lib
QT += core gui
CONFIG += debug
DEFINES += QT_DLL DATAVIEW_LIB
INCLUDEPATH += ./GeneratedFiles \
    . \
    ./../include \
    ./GeneratedFiles/Debug
PRECOMPILED_HEADER = StdAfx.h
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/debug
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
HEADERS += ./dataview_global.h \
    ./setting.h \
    ./mainui.h \
    ./dataview.h
SOURCES += ./dataview.cpp \
    ./mainui.cpp \
    ./setting.cpp
FORMS += ./mainui.ui \
    ./setting.ui

TRANSLATIONS += DataView_zh.ts\
			DataView_en.ts
