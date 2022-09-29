/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.cpp
* Heft: 18E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ctime>
#include <algorithm>

#include <QInputDialog>
#include <QMessageBox>

/***********************************************************************
* template <typename T> QSet<T> QListToQSet()
*
* Aufgabe 4
* Funktion: Ein Template für die Listenumwandlung.
***********************************************************************/
template <typename T> QSet<T> QListToQSet(const QList<T>& qlist)
{
  return QSet<T> (qlist.constBegin(), qlist.constEnd());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamen));
    //Zufallsgenerator initialisieren
    srand(time(NULL));
    //die Liste aktualisieren
    listeAktualisieren();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonNeu_clicked()
{
    //die Elemente an die Liste anhängen
    for (int i = 0; i < ui->spinBoxNeu->value(); i++)
        liste1.push_back(rand() % 100);
    listeAktualisieren();
}

void MainWindow::on_buttonNeuVor_clicked()
{
    //einen neuen Wert an der gewünschten Stelle einfügen
    liste1.insert(ui->spinBoxNeuVor->value() - 1, rand() % 100);
    //die Liste aktualisieren
    listeAktualisieren();
}
void MainWindow::on_buttonLoeschen_clicked()
{
    //den Wert an der gewünschten Stelle löschen
    liste1.erase(liste1.begin() + ui->spinBoxLoeschen->value() - 1);
    //die Liste aktualisieren
    listeAktualisieren();
}

void MainWindow::on_buttonAlleLoeschen_clicked()
{
    liste1.clear();
    listeAktualisieren();
}

void MainWindow::on_buttonSortieren_clicked()
{
    //die Liste sortieren
    std::sort(liste1.begin(), liste1.end());
    //Aufgabe 4: die zweite Liste sortieren
    std::sort(liste1New.begin(), liste1New.end());
    listeAktualisieren();
}

void MainWindow::on_buttonSuchen_clicked()
{

    int suchen, pos;
    //den gesuchten Wert über einen QInputDialog lesen
    suchen = QInputDialog::getInt(this, "Suchen", "Geben Sie den gesuchten Wert ein: ", -1);
    if (suchen != -1)
    {
        pos = liste1.indexOf(suchen);
        if (pos != -1)
            ui->listWidget->item(pos)->setSelected(true);
        //sonst eine Nachricht ausgeben
        else
            QMessageBox::information(this,"Hinweis", "Der Wert wurde nicht gefunden.");
    }
}

void MainWindow::on_buttonBeenden_clicked()
{
    //das Formular schließen
    close();
}

void MainWindow::listeAktualisieren()
{
    //die Liste leeren
    ui->listWidget->clear();
    foreach(int element, liste1)
        ui->listWidget->addItem(QString::number(element));

    //Aufgabe 4: Die zweite Liste leeren und mit den Elementen befühlen.
    ui->listWidget_2->clear();
    foreach(int element, liste1New)
        ui->listWidget_2->addItem(QString::number(element));

    //die maximalen Werte für die Drehfelder neu setzen
    ui->spinBoxLoeschen->setMaximum(liste1.size());
    ui->spinBoxNeuVor->setMaximum(liste1.size() + 1);
    //wenn die Liste leer ist, deaktivieren wir die Schaltflächen
    if (liste1.isEmpty())
    {
        ui->buttonAlleLoeschen->setEnabled(false);
        ui->buttonSortieren->setEnabled(false);
        ui->buttonSuchen->setEnabled(false);
        ui->buttonLoeschen->setEnabled(false);
        ui->buttonRemove->setEnabled(false);

        //wenn im Drehfeld für das Löschen 0 angezeigt wird, setzen
        //wir die Werte wieder auf 1
        if (ui->spinBoxLoeschen->value() == 0)
        {
            ui->spinBoxLoeschen->setMaximum(1);
            ui->spinBoxLoeschen->setValue(1);
        }
    }
    //sonst schalten wir die Schaltflächen ein
    else
    {
        ui->buttonAlleLoeschen->setEnabled(true);
        ui->buttonSortieren->setEnabled(true);
        ui->buttonSuchen->setEnabled(true);
        ui->buttonLoeschen->setEnabled(true);
        ui->buttonRemove->setEnabled(true);
    }
}

/***********************************************************************
* void MainWindow::on_buttonRemove_clicked()
*
* Aufgabe 4
* Funktion: Anhand eines Templates wird eine Liste vom Typ QList in
* eine andere Liste vom Typ QSet umgewandelt, dabei werden die Duplikate
* nicht mit kopiert, da diese Classe keine zulässt. Dadurch wird eine
* "Remove"-Funktion stillschweigend realisiert.
***********************************************************************/

void MainWindow::on_buttonRemove_clicked()
{
    QSet<int> aSet = QListToQSet(liste1);
    liste1New = aSet.values();
    ui->listWidget_2->clear();
    foreach(int element, liste1New)
        ui->listWidget_2->addItem(QString::number(element));
}

/*********************************************************************
* void MainWindow::on_buttonInput_clicked()
*
* Aufgabe 4
* Funktion: Hilfsfunktion.
* Ein Input-Dialog für die gezielte Eingabe von Duplikaten.
*********************************************************************/
void MainWindow::on_buttonInput_clicked()
{
    int valueIn;
    //den Wert über einen QInputDialog lesen
    valueIn = QInputDialog::getInt(this, "Eingeben", "Geben Sie gezielt ein Duplikat ein: ", -1);
    if (valueIn != -1)
    {
        liste1.push_back(valueIn);
        listeAktualisieren();
    }
}
