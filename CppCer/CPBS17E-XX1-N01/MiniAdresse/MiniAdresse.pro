QT       += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    einzelanzeige.cpp \
    listenanzeige.cpp \
    main.cpp \
    mainwindow.cpp \
    neuereintrag.cpp \
    suchfunktion.cpp

HEADERS += \
    einzelanzeige.h \
    listenanzeige.h \
    mainwindow.h \
    neuereintrag.h \
    suchfunktion.h

FORMS += \
    einzelanzeige.ui \
    mainwindow.ui \
    neuereintrag.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
