QT       += core gui

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about.cpp \
    edb/CComHelper.cpp \
    edb/EDBInterface.cpp \
    edb/WinReg.cpp \
    flashinginfo.cpp \
    main.cpp \
    options.cpp \
    startwindow.cpp \
    updatewindow.cpp \
    wait.cpp

HEADERS += \
    about.h \
    edb/CComHelper.h \
    edb/EDBInterface.h \
    edb/WinReg.h \
    flashinginfo.h \
    libusb/libusb.h \
    options.h \
    startwindow.h \
    updatewindow.h \
    wait.h

FORMS += \
    about.ui \
    flashinginfo.ui \
    options.ui \
    startwindow.ui \
    updatewindow.ui \
    wait.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
