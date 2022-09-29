/*******************************************************************
* Einsendeaufgabe
* Datei: suchfunktion.cpp
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "suchfunktion.h"
#include "einzelanzeige.h"
#include "listenanzeige.h"
#include <QTableView>
#include "ui_suchfunktion.h"
//der Konstruktor
//er erstellt den kompletten Inhalt
/*******************************************************************
* suchFunktion::suchFunktion()
*
* Aufgabe 2
* Funktion: Die Ergebnisse der Suche werden in einer Tabelle in einem
* weiteren Formular gefiltert dargestellt.
*******************************************************************/
suchFunktion::suchFunktion(QString setFilter)
{
    //den Fenstertitel setzen
    setWindowTitle("Listenanzeige nach Suchkriterium");
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

    //Aufgabe 2
    //Funktion: Aufbau vom Filter (Suchparameter).
    modell->setFilter("anachname = '" + setFilter + "'");

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
