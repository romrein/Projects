/*########################################
Einsendeaufgabe 11.3
##########################################*/

//die Header-Dateien einbinden
#include "bildbetrachter.h"
#include <QPixmap>
#include <QMessageBox>
#include <QFileDialog>

#define CLOSEIT 0x00004000      //Konstante für Standarttaste "Yes"

//der Konstruktor
Bildbetrachter::Bildbetrachter()
{
        triggerBild = false;
        timerZeit = new QTimer(this);
        //die Größe und den Titel setzen
        resize(250,250);
        setWindowTitle("Smiley");

        //die Widgets erzeugen und positionieren
        //Informationszeile für interne Debugging
        einLabel = new QLabel(this);
        einLabel->setGeometry(90, 250, 500, 10);

        //"Platzhalter" bzw. Anzeigecontainer für die Bilder
        bildLabel = new QLabel(this);
        bildLabel->setGeometry(50, 30, 10, 10);

        //der Beenden-Button
        beendenButton = new QPushButton(this);
        beendenButton->setGeometry(90, 200, 70, 30);
        beendenButton->setText("Beenden");
        //Signal mit dem Slot verbinden
        QObject::connect(beendenButton, SIGNAL(clicked()), this, SLOT(ifClose()));

        //Aktuelles Arbeitsverzeichnis, wo die Bilder abgespeichert sind, setzen
        if (QDir::setCurrent("../bildbetrachter"))
        {
            einLabel->setText(QDir::currentPath());
            qPath = QDir::currentPath();
        }
        else
        {
            einLabel->setText("ERR");
        }

        //Timer für die Bilderaustausch definieren
        timerZeit->start(1000);
        //Signal mit dem Slot verbinden
        QObject::connect(timerZeit, SIGNAL(timeout()), this, SLOT(toggelnBilddatei()));
}

//User Interface zum Schließen der Anwendung
void Bildbetrachter::ifClose()
{
    if ((QMessageBox::question(this, "Close", "Sind Sie sicher?\n")) == CLOSEIT)
        close();
}

//Slot zum Bildwechsel
void Bildbetrachter::toggelnBilddatei()
{
    QString DateiName1, DateiName2;
    static int ret;

    //Komplette Pfad zu den Bildern
    DateiName1 = qPath + "/Bild1";
    DateiName2 = qPath + "/Bild2";

    //Zustandswechsel
    triggerBild = !triggerBild;

    //Routine zum Bildwechsel und möglichen Fehlerbehandlung
    if (ret == 0)
    {
        if (triggerBild == false)
        {
            ret = oeffneBilddatei(DateiName1);
            //Info zum Debugging
            einLabel->setText(DateiName1);
        }
        else
        {
            ret = oeffneBilddatei(DateiName2);
            //Info zum Debugging
            einLabel->setText(DateiName2);
        }
    }
    else
    {
        //Abfrage des Speicherortes im Fehlerfall
        qPath = QFileDialog::getExistingDirectory();
        ret = 0;
    }
}

//Slot zum Öffnen den Bildern
int Bildbetrachter::oeffneBilddatei(QString FileName)
{
    QPixmap bild;
    //konnte das Bild geladen werden?
    //dann anzeigen
    if (bild.load(FileName))
    {
        //die Größe wird dabei an die Breite 150 angepasst
        bild = bild.scaledToWidth(150);
        //im Label anzeigen
        bildLabel->setPixmap(bild);
        //die Größe des Labels an die Größe des Bildes anpassen
        bildLabel->resize(bild.size());
    }
    else
    {
        QMessageBox::warning(this, "Fehler beim Öffnen der Datei", "Die Grafikdatei konnte nicht geöffnet werden.\n");
        einLabel->setText(FileName);
        return -1;
    }
    return 0;
}
