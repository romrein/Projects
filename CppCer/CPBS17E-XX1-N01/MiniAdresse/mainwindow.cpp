/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.cpp
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
//die Header-Datei für das Formular
#include "neuereintrag.h"
#include "listenanzeige.h"
#include "einzelanzeige.h"
#include "suchfunktion.h"

#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamen));
    this->setFixedSize(413,110);
    dbVerbunden = dbVerbinden();
}
MainWindow::~MainWindow()
{
    delete ui;
}
//die Methode stellt die Verbindung zur Datenbank her
bool MainWindow::dbVerbinden()
{
    bool gelungen = true;
    //die Verbindung zur Datenbank herstellen
    datenbank = QSqlDatabase::addDatabase("QSQLITE");

    //Aufgabe 1/2
    //Beachte: Überprüfen, ob der richtige Pfad eingegeben ist
    //datenbank.setDatabaseName("c:/Qt/minidb.db");
    datenbank.setDatabaseName("c:/Qt/Projekte/CPBS17E/minidb.db");

    //konnte die Datenbank geöffnet werden?
    //wenn nicht, geben wir eine Meldung aus
    if (datenbank.open() == false)
    {
        //den Fehler beschaffen wir uns im Klartext
        QString fehler = datenbank.lastError().text();
        //bitte in einer Zeile eingeben
        QMessageBox::critical(this, "Fehler", "Die Datenbank konnte nicht geöffnet werden.\nGrund: " + fehler);
        gelungen = false;
    }
    return gelungen;
}
//die Slots
void MainWindow::on_buttonListenAnzeige_clicked()
{
    //wenn eine Verbindung zu Datenbank besteht
    if (dbVerbunden == true)
    {
        //das Formular anzeigen
        listenAnzeige *formNeu = new listenAnzeige();
        formNeu->show();
    }
}
void MainWindow::on_buttonEinzelAnzeige_clicked()
{
    //wenn eine Verbindung zu Datenbank besteht
    if (dbVerbunden == true)
    {
        //das Formular anzeigen
        einzelAnzeige *formNeu = new einzelAnzeige();
        formNeu->show();
    }
}
void MainWindow::on_buttonNeuerEintrag_clicked()
{
    //wenn eine Verbindung zu Datenbank besteht
    if (dbVerbunden == true)
    {
        //das Formular anzeigen
        neuerEintrag *formNeu = new neuerEintrag();
        formNeu->show();
    }
}
void MainWindow::on_buttonBeenden_clicked()
{
    //die Datenbankverbindung schließen
    datenbank.close();
    //und auch das Formular
    this->close();
}

/*********************************************************************
* void MainWindow::on_buttonSuchen_clicked()
*
* Aufgabe 2
* Funktion: Bei der Betätigung der Schaltfläche im Hauptfenster wird
* zuerst ein Formular zur Eingabe des Suchkriteriums erscheinen. Der
* Eingabedialog ist vereinfacht mit der Klasse QInputDialog realisiert.
* Der Rückgabewert vom getText() wird als QString Parameter an die
* Suchfunktion übergeben und stellt somit ein Suchkriterium dar.
* Zusätzlich wird eine leere Eingabe und "Abrechen"-Wunsch vom Benutzer
* detektiert um die unnötige Anzeige von "leeren" Daten zu vermeiden.
*********************************************************************/
void MainWindow::on_buttonSuchen_clicked()
{
    bool ok;
    QString suchFilter;
    suchFilter = QInputDialog::getText(this, tr("Suchfunktion"),
          tr("Eingabe des Suchkriteriums:"), QLineEdit::Normal,
          "Nachname", &ok,
          windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));

    if (ok && !suchFilter.isEmpty())
    {
        //wenn eine Verbindung zu Datenbank besteht
        if (dbVerbunden == true)
        {
            //das Formular anzeigen
            suchFunktion *formNeu = new suchFunktion(suchFilter);
            formNeu->show();
        }
    }
}
