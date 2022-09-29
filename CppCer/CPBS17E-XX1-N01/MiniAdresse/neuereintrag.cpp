/*******************************************************************
* Einsendeaufgabe
* Datei: neuereintrag.cpp
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "neuereintrag.h"
#include "ui_neuereintrag.h"
//für die Messagebox
#include <QMessageBox>
neuerEintrag::neuerEintrag(QWidget *parent) :
QWidget(parent),
ui(new Ui::neuerEintrag)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamenForm));
    this->setFixedSize(260,260);
    modell = new QSqlTableModel(this);
    //die Tabelle setzen
    modell->setTable("adressen");
}
neuerEintrag::~neuerEintrag()
{
    delete ui;
}
//die Slots
void neuerEintrag::on_buttonOK_clicked()
{
    //eine leere Zeile ergänzen
    int zeile;
    zeile = modell->rowCount();
    modell->insertRow(zeile);
    //die Daten aus dem Formular übernehmen
    //bitte jeweils in einer Zeile eingeben
    modell->setData(modell->index(zeile, 1), ui->lineEditVorname->text());
    modell->setData(modell->index(zeile, 2), ui->lineEditName->text());
    modell->setData(modell->index(zeile, 3), ui->lineEditStrasse->text());
    modell->setData(modell->index(zeile, 4), ui->lineEditPLZ->text());
    modell->setData(modell->index(zeile, 5), ui->lineEditOrt->text());
    modell->setData(modell->index(zeile, 6), ui->lineEditTelefon->text());
    //die Änderungen übernehmen
    modell->submitAll();
    //eine Meldung anzeigen
    //bitte in einer Zeile eingeben
    QMessageBox::information(this,"Info", "Die Daten wurden übernommen.");
    //und alle Felder wieder leeren
    ui->lineEditName->clear();
    ui->lineEditVorname->clear();
    ui->lineEditStrasse->clear();
    ui->lineEditPLZ->clear();
    ui->lineEditOrt->clear();
    ui->lineEditTelefon->clear();
}
void neuerEintrag::on_buttonBeenden_clicked()
{
    //das Formular schließen
    this->close();
}
