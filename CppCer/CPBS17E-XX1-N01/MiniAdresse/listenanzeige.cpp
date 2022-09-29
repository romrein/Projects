/*******************************************************************
* Einsendeaufgabe
* Datei: listenanzeige.cpp
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
//die Header-Datei einbinden
#include "listenanzeige.h"
#include <QTableView>
//der Konstruktor
//er erstellt den kompletten Inhalt
listenAnzeige::listenAnzeige()
{
    //den Fenstertitel setzen
    setWindowTitle("Listenanzeige");
    this->setWindowIcon(QIcon(iconNamenForm));
    //das Modell erstellen
    QSqlTableModel *modell = new QSqlTableModel(this);
    //die Tabelle setzen
    modell->setTable("adressen");
    //die Texte für die Spaltenköpfe setzen
    modell->setHeaderData(0, Qt::Horizontal, "ID");
    modell->setHeaderData(1, Qt::Horizontal, "Vorname");
    modell->setHeaderData(2, Qt::Horizontal, "Name");
    modell->setHeaderData(3, Qt::Horizontal, "Strasse");
    modell->setHeaderData(4, Qt::Horizontal, "PLZ");
    modell->setHeaderData(5, Qt::Horizontal, "Ort");
    modell->setHeaderData(6, Qt::Horizontal, "Telefon");

    //die Daten beschaffen
    modell->select();
    //die TableView erstellen
    QTableView *ansicht = new QTableView(this);
    //mit dem Modell verbinden
    ansicht->setModel(modell);
    //die Breite der Spalten an den Inhalt anpassen
    ansicht->resizeColumnsToContents();
    //die Größe setzen
    ansicht->resize(500,300);
}


