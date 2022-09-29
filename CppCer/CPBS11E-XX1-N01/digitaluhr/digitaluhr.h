/*########################################
Einsendeaufgabe 11.2
##########################################*/

#ifndef DIGITALUHR_H
#define DIGITALUHR_H
//die Header-Dateien
#include <QLCDNumber>
#include <QTimer>
#include <QMouseEvent>

//Die Klasse Digitialuhr erbt von QLCDNumber
class Digitaluhr : public QLCDNumber
{
    //das Makro Q_OBJECT
    Q_OBJECT

    public:
        //der Konstruktor
        Digitaluhr();

    //die Methode für das Mausereignis
    protected:
        void mousePressEvent(QMouseEvent *event);

    //die Methoden
    private:
        void zeigeUhrzeit();
        void zeigeDatum();
        void stopDatum();

        bool doppelpunkt;
        //der erste Timer
        QTimer* timerZeit;
        //der zweite Timer
        QTimer* timerDatum;

    private slots:
        //die Methode für den Timer
        void timerZeitSlot();
        //die Methode für den zweiten Timer
        void timerDatumSlot();
};
#endif
