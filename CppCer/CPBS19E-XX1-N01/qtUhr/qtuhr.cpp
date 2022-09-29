/*******************************************************************
* Einsendeaufgabe
* Datei: qtuhr.cpp
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "qtuhr.h"
#include <QTime>
#include <QDateTime>

qtUhr::qtUhr(QObject *parent) : QObject(parent)
{
    zeitAct = "00:00:00:00";
}

/*********************************************************************
* QString qtUhr::liefereZeit()
*
* Aufgabe 3
* Funktion: Die aktuelle Zeit wird ermittelt und an die aufrufende
* Funktion zurück geliefert.
*********************************************************************/
QString qtUhr::liefereZeit() {
    //die Zeit abfragen und aufbereiten
    zeitAct = QTime::currentTime().toString().left(8);
    return zeitAct;
}

/*********************************************************************
* QString qtUhr::liefereDatum()
*
* Aufgabe: Das aktuelle Datum wird ermittelt und an die aufrufende
* Funktion zurück geliefert.
*********************************************************************/
QString qtUhr::liefereDatum() {
    datumAct = QDate::currentDate().toString("dd.MM.yyyy");
    return datumAct;
}
