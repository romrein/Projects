/*******************************************************************
* Einsendeaufgabe
* Datei: suchfunktion.h
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef SUCHFUNKTION_H
#define SUCHFUNKTION_H

#include <QDialog>
#include <QInputDialog>

//unsere Klasse erbt von QDialog
class suchFunktion : public QDialog
{
public:
    //der Konstruktor
    //Aufgabe 2
    //Deklaration von der Methode mit einem Parameter vom Typ QString.
    suchFunktion(QString setFilter);
    QString iconNamenForm =  ":/Bild1.png";
};

#endif // SUCHFUNKTION_H
