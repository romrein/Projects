/*########################################
Einsendeaufgabe 11.2
##########################################*/

//die Header-Dateien einbinden
#include "digitaluhr.h"
#include <QDateTime>
//der Konstruktor
Digitaluhr::Digitaluhr()
{
    doppelpunkt = false;
    //den Segmentstyle setzen
    setSegmentStyle(QLCDNumber::Filled);
    //den Timer erzeugen
    timerZeit = new QTimer(this);
    //das Signal timeout des Timers mit dem Slot verbinden
    QObject::connect(timerZeit, SIGNAL(timeout()), this, SLOT(timerZeitSlot()));
    //den Timer starten
    timerZeit->start(1000);
    //die Uhrzeit anzeigen
    zeigeUhrzeit();
    //den zweiten Timer erzeugen und verbinden
    //der Timer wird aber noch nicht gestartet!
    timerDatum = new QTimer(this);
    QObject::connect(timerDatum, SIGNAL(timeout()), this, SLOT(timerDatumSlot()));

}

//die Methode für das Mausereignis
void Digitaluhr::mousePressEvent(QMouseEvent *event)
{
    //wurde die linke Maustaste gedrückt?
    if (event->button() == Qt::LeftButton)
        //dann das Datum anzeigen
        zeigeDatum();
}

//die Methode für den Timer der Zeit
void Digitaluhr::timerZeitSlot()
{
    //die Uhrzeit anzeigen
    zeigeUhrzeit();
}

//die Methode für den Timer des Datums
void Digitaluhr::timerDatumSlot()
{
    //die Methode stopDatum() aufrufen
    stopDatum();
}

//die Methode zeigeUhrzeit()
void Digitaluhr::zeigeUhrzeit()
{
    //die Zeit abfragen und aufbereiten
    QString zeitAnzeige;
    zeitAnzeige = QTime::currentTime().toString().left(8);
    //den Status des Doppelpunkts ändern
    if (doppelpunkt == false)
        doppelpunkt = true;
    else
        doppelpunkt = false;
    //den Doppelpunkt abschalten
    if (doppelpunkt == false)
    {
        zeitAnzeige[2] = ' ';
        zeitAnzeige[5] = ' ';
    }
    setDigitCount(8);
    display(zeitAnzeige);
}

//die Methode zeigeDatum()
void Digitaluhr::zeigeDatum()
{
    //läuft der Timer für das Datum noch?
    //dann verlassen wir die Methode direkt wieder
    QString datumAnzeige;
    if (timerDatum->isActive() == true)
        return;
    //das Datum aufbereiten und anzeigen
    datumAnzeige = QDate::currentDate().toString("dd.MM.yyyy");
    setDigitCount(10);
    display(datumAnzeige);
    //den Timer für die Zeit anhalten
    //sonst verschwindet das Datum nach 1 Sekunde wieder
    timerZeit->stop();
    //den Timer für das Datum starten
    timerDatum->start(2000);
}

//die Methode stopDatum()
void Digitaluhr::stopDatum()
{
    //den Timer für die Uhr anhalten
    timerDatum->stop();
    //den Timer für die Uhrzeit wieder starten
    timerZeit->start(1000);
    //die Uhrzeit wieder anzeigen
    setDigitCount(8);
    zeigeUhrzeit();
}
