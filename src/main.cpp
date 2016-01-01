/** 
 * \file main.cpp
 * \brief main
 * \author Yves Ouvrard
 * \version 11
 * \date 2016
 *
 * main
 */

#include <QApplication>

#include "mainwindow.h"

/**
 * \fn main (int argc, char *argv[])
 * \brief fonction principale de l'application.
 */
int main (int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
