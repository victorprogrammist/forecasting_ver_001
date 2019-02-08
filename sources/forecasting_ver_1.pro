#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T12:55:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = forecasting_ver_1
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tls.cpp \
    file_loader.cpp \
    corr_slide.cpp \
    dispersion.cpp \
    f_main.cpp \
    cicling.cpp \
    f_prepare_by_date_time.cpp \
    corr.cpp \
    f_select_best_node.cpp \
    f_calc_corr.cpp \
    f_src_data.cpp \
    f_auxiliary.cpp

HEADERS += \
        mainwindow.h \
    tls.h \
    dispersion.h \
    ord_pack.h \
    file_loader.h \
    corr_slide.h \
    corr.h \
    outlier.h \
    cicling.h \
    f_main.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder

CONFIG += precompile_header
