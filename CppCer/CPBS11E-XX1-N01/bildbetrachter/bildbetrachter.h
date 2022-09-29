/*########################################
Einsendeaufgabe 11.3
##########################################*/

#ifndef BILDBETRACHTER_H
#define BILDBETRACHTER_H
//die Header-Dateien einbinden
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

//unsere eigene Klasse erbt von QWidget
class Bildbetrachter : public QWidget
{
    //das Makro Q_OBJECT
    Q_OBJECT
    public:
        //der Konstruktor
        Bildbetrachter();

    //die Slots
    private slots:
        void toggelnBilddatei();
        void ifClose();
        int oeffneBilddatei(QString);

    private:
        //die Attribute für die Widgets
        bool triggerBild;
        QLabel* einLabel, *bildLabel;
        QPushButton *beendenButton;
        QTimer *timerZeit;
        QString qPath;
};
#endif
