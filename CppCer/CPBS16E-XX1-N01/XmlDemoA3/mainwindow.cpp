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

//für die MessageBox
#include <QMessageBox>
//für QXmlStreamReader und QXmlStreamWriter
#include <QtXml/QtXml>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamen));
    //Aufgabe 3
    //Funktion: Datei wird im aktuellen Pfad der Anwendung gespeichert
    xmlName = QCoreApplication::applicationFilePath() + ".xml";

    datei = new QFile(xmlName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*******************************************************************
* void MainWindow::on_buttonSchreiben_clicked()
*
* Aufgabe 3
* Funktion: Beim Anklicken der Taste werden die Einträge von einem
* Listenfeld in einer XML-Datei gespeichert.
*******************************************************************/
void MainWindow::on_buttonSchreiben_clicked()
{
    //die Datei zum Schreiben öffnen
    datei->open(QIODevice::WriteOnly);
    QString sIndex;
    //den XmlStreamWriter aus der Datei erzeugen
    QXmlStreamWriter *xmlWriter = new QXmlStreamWriter(datei);
    //die automatische Formatierung aktivieren
    xmlWriter->setAutoFormatting(true);

    //die XML-Deklaration schreiben
    xmlWriter->writeStartDocument();
    //den Wurzelknoten erstellen
    xmlWriter->writeStartElement("daten");
    //das Unterelement werte schreiben
    xmlWriter->writeStartElement("Eintraege");
    //die Einträge schreiben

    //Aufgabe 3
    //Funktion: Die Einträge von einem Listenfeld werden in einer XML-Datei
    //gespeichert. Zusätzlich werden die Knoten Namen mit einer laufenden der
    //Zeile entsprechenden Nummer versehen.
    for (int index = 0; index < ui->listWidget->count(); index++)
    {
        ui->listWidget->setCurrentRow(index);
        sIndex = "Eintrag_"+(QString::number(index));
        xmlWriter->writeTextElement(sIndex, ui->listWidget->currentItem()->text());
    }

    xmlWriter->writeEndElement();

    //den Wurzelknoten abschließen
    xmlWriter->writeEndElement();
    //das Dokument abschließen
    xmlWriter->writeEndDocument();
    //die Datei wieder schließen
    datei->close();
}

/*******************************************************************
* void MainWindow::on_pushButtonList_clicked()
*
* Aufgabe 3
* Funktion: Die Liste wird vom Anwender mit den Daten befüllt.
*******************************************************************/
void MainWindow::on_pushButtonList_clicked()
{
    ui->listWidget->addItem(ui->lineEdit->text());
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}
