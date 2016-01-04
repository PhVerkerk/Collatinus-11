VERSION = "11.0"
DEFINES += VERSION=\\\"$$VERSION\\\"

TEMPLATE = app
TARGET = collatinus
INCLUDEPATH += . src
DEPENDPATH += .

qtHaveModule(printsupport): QT += printsupport
QT += widgets
QT += xmlpatterns
QT += network
QT += svg

CONFIG += release_binary


unix:!macx:DESTDIR = bin
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

macx:{
    TARGET = Collatinus
    #note mac os x, fair un $ qmake -spec macx-g++
    #CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
    ICON = collatinus.icns
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk

    #QMAKE_POST_LINK=strip Collatinus.app/Contents/MacOS/collatinus

    # install into app bundle
    # à changer en ressources
#    data.path = Collatinus.app/Contents/MacOS
#    data.files =  ressources/*
#    deploy.depends = install_documentation
#    deploy.depends += install
#    documentation.path = Collatinus.app/Contents/MacOS/doc/
#    documentation.files = doc/*.html doc/*.css
    # ajouter un cible qui fait macdeploy Collatinus.app
    deploy.commands = macdeployqt Collatinus.app
#    dmg.depends = deploy
#	dmg.commands = ./MacOS/Collatinus.sh
#    INSTALLS += documentation
#    INSTALLS += data
    QMAKE_EXTRA_TARGETS += deploy
#    QMAKE_EXTRA_TARGETS += dmg
}
