VERSION = "11.0"
DEFINES += VERSION=\\\"$$VERSION\\\"

TEMPLATE = app
TARGET = collatinus
INCLUDEPATH += . src

qtHaveModule(printsupport): QT += printsupport
QT += widgets
QT += xmlpatterns
QT += network

DESTDIR = bin
OBJECTS_DIR= obj/
MOC_DIR = moc/
QMAKE_DISTCLEAN += $${DESTDIR}/collatinus

# Input
HEADERS += src/ch.h \
           src/flexion.h \
           src/irregs.h \
           src/lemmatiseur.h \
           src/lemme.h \
           src/lewis.h \
           src/mainwindow.h \
           src/modele.h
SOURCES += src/ch.cpp \
           src/flexion.cpp \
		   src/frequences.cpp \
           src/irregs.cpp \
           src/lemmatiseur.cpp \
           src/lemme.cpp \
           src/lewis.cpp \
           src/main.cpp \
           src/mainwindow.cpp \
           src/modele.cpp \
           src/scandeur.cpp

RESOURCES += collatinus.qrc
