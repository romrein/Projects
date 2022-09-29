/*########################################
Einsendeaufgabe 11.2
##########################################*/

#include <QApplication>
//die Header-Datei der eigenen Klasse einbinden
#include "digitaluhr.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //ein neue Instanz der Klasse Digitaluhr erzeugen
    Digitaluhr *meineUhr = new Digitaluhr();
    //die Größe anpassen
    meineUhr->resize(300, 100);
    //den Titel setzen
    meineUhr->setWindowTitle("Digitaluhr");
    //das Formular anzeigen
    meineUhr->show();
    return app.exec();
}
