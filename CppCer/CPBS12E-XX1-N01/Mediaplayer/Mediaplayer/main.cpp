/*******************************************************************
* Einsendeaufgabe
* Datei: main.cpp
* Heft: 12E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //w.showMaximized();
    return a.exec();
}
