#
# Тестовое задание
# requare: Qt5.
#

QT = core gui network widgets multimedia multimediawidgets

HEADERS += \
    serverintf.h \
    inetfile.h \
    filmlistview.h \
    filmpreviewer.h \
    filmlistitem.h \
    mediaplayer.h

SOURCES += \
    serverintf.cpp \
    main.cpp \
    inetfile.cpp \
    filmlistview.cpp \
    filmpreviewer.cpp \
    filmlistitem.cpp \
    mediaplayer.cpp


TARGET = review_of_films
DESTDIR = bin
OBJECTS_DIR = .buld/
MOC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR


#win32: QMAKE_DISTCLEAN += /F /S .build bin
QMAKE_DISTCLEAN += -r .build bin

FORMS += \
    filmpreviewer.ui

#RESOURCES += resources.qrc
