/*******************************************************************
* Einsendeaufgabe
* Datei: listenanzeige.h
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef LISTENANZEIGE_H
#define LISTENANZEIGE_H
//die Dateien einbinden
//für die SQL-Klassen
#include <QtSql>
#include <QtGui>
#include <QDialog>
//unsere Klasse erbt von QDialog
class listenAnzeige : public QDialog
{
public:
    //der Konstruktor
    listenAnzeige();
    QString iconNamenForm =  ":/Bild1.png";
};
#endif
