/*******************************************************************
* Einsendeaufgabe
* Datei: einzelanzeige.cpp
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "einzelanzeige.h"
#include "ui_einzelanzeige.h"
#include <QtSql>
//der Konstruktor
//er verbindet das Formular mit der Oberfläche und
//stellt die Verbindungen her
einzelAnzeige::einzelAnzeige(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::einzelAnzeige)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamenForm));
    datenSatzLbl = new QLabel("0 von 0", this);
    datenSatzLbl->setGeometry(20, 280, 150, 10);
    this->setFixedSize(260,320);
    verbindungenHerstellen();
    datenMappen();
}
einzelAnzeige::~einzelAnzeige()
{
    delete ui;
}
void einzelAnzeige::verbindungenHerstellen()
{
    //die Verbindungen herstellen
    //bitte jeweils in einer Zeile eingeben
    connect(ui->actionGanz_nach_vorne, SIGNAL(triggered(bool)), this, SLOT(ganzNachVorne()));
    connect(ui->actionEinen_nach_vorne, SIGNAL(triggered(bool)), this, SLOT(einenNachVorne()));
    connect(ui->actionEinen_nach_hinten, SIGNAL(triggered(bool)), this, SLOT(einenNachHinten()));
    connect(ui->actionGanz_nach_hinten, SIGNAL(triggered(bool)), this, SLOT(ganzNachHinten()));
    connect(ui->actionL_schen, SIGNAL(triggered(bool)), this, SLOT(loeschen()));
}
void einzelAnzeige::datenMappen()
{
    //das Modell erstellen
    modell = new QSqlTableModel(this);
    //die Tabelle setzen
    modell->setTable("adressen");
    //die Daten beschaffen
    modell->select();
    //die Verbindungen zwischen den Widgets und den Daten
    //herstellen
    mapper = new QDataWidgetMapper(this);
    mapper->setModel(modell);
    mapper->addMapping(ui->lineEditID, 0);
    mapper->addMapping(ui->lineEditVorname, 1);
    mapper->addMapping(ui->lineEditName, 2);
    mapper->addMapping(ui->lineEditStrasse, 3);
    mapper->addMapping(ui->lineEditPLZ, 4);
    mapper->addMapping(ui->lineEditOrt, 5);
    mapper->addMapping(ui->lineEditTelefon, 6);
    //Änderungen sollen automatisch übernommen werden
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    //zum ersten Datensatz gehen
    mapper->toFirst();

    //Aufgabe 1
    //Funktion: Dieses Signal wird ausgegeben, nachdem sich der aktuelle Index
    //geändert hat und alle Widgets mit neuen Daten belegt wurden. Empfänger
    //ist ein Slot, der die Liste aktualisiert.
    connect(mapper ,&QDataWidgetMapper::currentIndexChanged,this,&einzelAnzeige::indexRefresh);
    indexRefresh();
}
//die Slots
//ganz nach vorne springen
void einzelAnzeige::ganzNachVorne()
{
    mapper->toFirst();
}
//einen nach vorne springen
void einzelAnzeige::einenNachVorne()
{
    mapper->toPrevious();
}
//einen nach hinten springen
void einzelAnzeige::einenNachHinten()
{
    mapper->toNext();
}
//ganz nach hinten springen
void einzelAnzeige::ganzNachHinten()
{
    mapper->toLast();
}
//einen Datensatz löschen
void einzelAnzeige::loeschen()
{
    //den aktuellen Index merken
    int index = mapper->currentIndex();
    //den Eintrag löschen
    modell->removeRow(mapper->currentIndex());
    //die Änderungen übernehmen
    mapper->submit();
    //die Daten neu beschaffen
    modell->select();
    //wieder zum ursprünglichen Datensatz gehen
    //aber nur dann, wenn nicht der letzte gelöscht wurde
    //wenn der letzte Datensatz gelöscht wurde, gehen wir
    //zum neuen "letzten"
    if (index < modell->rowCount())
        mapper->setCurrentIndex(index);
    else
        mapper->toLast();
}

/*********************************************************************
* void einzelAnzeige::indexRefresh()
*
* Aufgabe 1
* Funktion: Die Nummer des aktuellen Datensatzes und die Anzahl aller
* Datensätze in der Datenbanktabelle werden über Signal-Slot Verbindung
* aktualisiert und in einem Label angezeigt.
*********************************************************************/
void einzelAnzeige::indexRefresh()
{
    QString count, cntMax;
    count = QString::number(mapper->currentIndex()+1);
    cntMax = QString::number(modell->rowCount());
    datenSatzLbl->setText("Datensatz "+count+" von "+cntMax);
}
