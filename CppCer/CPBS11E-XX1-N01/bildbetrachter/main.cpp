/*########################################
Einsendeaufgabe 11.3
##########################################*/

#include <QApplication>
#include "bildbetrachter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //das Haupt-Widget erzeugen und anzeigen
    Bildbetrachter* betrachter = new Bildbetrachter();
    betrachter->show();
    return app.exec();
}
