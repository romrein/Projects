/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.cpp
* Heft: 16E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamen));
    //die Verbindungen herstellen
    //die ersten drei gehen jeweils auf einen eigenen Slot
    //die letzte nutzt einen Standard-Slot
    connect(ui->action_Neu, SIGNAL(triggered(bool)), this, SLOT(dateiNeu()));
    connect(ui->action_ffnen, SIGNAL(triggered(bool)), this, SLOT(dateiOeffnen()));
    connect(ui->action_Speichern, SIGNAL(triggered(bool)), this, SLOT(dateiSpeichern()));
    connect(ui->action_Beenden, SIGNAL(triggered(bool)), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//die Slots
//zum "Anlegen" einer neuen Datei
void MainWindow::dateiNeu()
{
    //die Abfrage
    QMessageBox meineAbfrage(QMessageBox::Question, "Abfrage", "Wollen Sie wirklich eine neue Datei anlegen?", QMessageBox::Yes | QMessageBox::No);
    meineAbfrage.setButtonText(QMessageBox::Yes, "Ja");
    meineAbfrage.setButtonText(QMessageBox::No, "Nein");
    //die Abfrage ausführen und auswerten
    //war die Antwort Ja?
    if (meineAbfrage.exec() == QMessageBox::Yes)
        ui->textEdit->clear();
}

//zum Öffnen der Datei
void MainWindow::dateiOeffnen()
{
    //den Öffnendialog erzeugen
    dateiName = QFileDialog::getOpenFileName(this, "Datei öffnen", QDir::currentPath(), "Textdateien (*.txt)");
    //wenn eine Datei ausgewählt wurde
    if (dateiName.isEmpty() == false)
    {
        //eine Instanz von QFile für die Datei erzeugen
        QFile *datei = new QFile(dateiName);
        //die Datei im Modus Nur-Lesen öffnen
        if (datei->open(QIODevice::ReadOnly))
        {
            //den kompletten Inhalt laden und in das Widget setzen
            ui->textEdit->setPlainText(QString::fromUtf8 (datei->readAll()));
            //eine Meldung in der Statusleiste anzeigen
            ui->statusBar->showMessage("Die Datei wurde geladen.", 5000);
            //die Datei wieder schließen
            datei->close();
        }
    }
}

/*******************************************************************
* void MainWindow::dateiSpeichern()
*
* Aufgabe 2
* Funktion: Speichern einer Datei. Dabei wird der Name nur einmal
* abgefragt.
*******************************************************************/
void MainWindow::dateiSpeichern()
{
    if (QFile::exists(dateiName) == false)
    {
        //den Speichern-Dialog erzeugen
        dateiName = QFileDialog::getSaveFileName(this, "Datei speichern", QDir::currentPath(), "Textdateien (*.txt)");
    }
    else
    {
        //Die Datei existiert bereits. Der vorhandene Name verwenden.
    }

    //wenn eine Datei ausgewählt wurde
    if (dateiName.isEmpty() == false)
    {
        //Der Name an Hilfsfunktion zum Speichern übergeben.
        dateiSpeichernUnter(dateiName);
    }
}

/*******************************************************************
* void MainWindow::on_actionInfo_triggered()
*
* Aufgabe 1
* Funktion: Dialog Informationen zum Programm wird anzeigt.
*******************************************************************/
void MainWindow::on_actionInfo_triggered()
{
    QMessageBox::information(this, "Information", "Programmiert von Wjatscheslaw Rein");
}

/*******************************************************************
* void MainWindow::on_actionSpeichern_unter_triggered()
*
* Aufgabe 2
* Funktion: Der Slot für die Aktion Datei Speichern Unter...
*******************************************************************/
void MainWindow::on_actionSpeichern_unter_triggered()
{
    //den Speichern-Dialog erzeugen
    dateiName = QFileDialog::getSaveFileName(this, "Datei speichern", QDir::currentPath(), "Textdateien (*.txt)");
    //Der Name an Hilfsfunktion zum Speichern übergeben.
    dateiSpeichernUnter(dateiName);
}

/*******************************************************************
* void MainWindow::dateiSpeichernUnter()
*
* Aufgabe 2
* Funktion: Eine verlagerte Hilfsfunktion zum Speichern.
* Die Datei wird unter vorgegebenem Namen gespeichert.
*******************************************************************/
void MainWindow::dateiSpeichernUnter(QString dateiNameToSave)
{
    //eine Instanz von QFile für die Datei erzeugen
    QFile *datei = new QFile(dateiNameToSave);
    //die Datei im Modus Nur-Schreiben öffnen
    if (datei->open(QIODevice::WriteOnly))
    {
        //den kompletten Inhalt aus dem Widget in die Datei //schreiben
        datei->write(ui->textEdit->toPlainText().toUtf8());
        //eine Meldung in der Statusleiste anzeigen
        ui->statusBar->showMessage("Die Datei wurde gespeichert.", 5000);
        //die Datei wieder schließen
        datei->close();
    }
}
