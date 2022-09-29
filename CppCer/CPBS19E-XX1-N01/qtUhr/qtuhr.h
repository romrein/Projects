/*******************************************************************
* Einsendeaufgabe
* Datei: qtuhr.h
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef QTUHR_H
#define QTUHR_H

#include <QObject>
#include <QString>
#include <QTimer>

class qtUhr : public QObject
{
    Q_OBJECT
    //die Makros für die Verbindungen
    Q_PROPERTY(QString zeitAct READ liefereZeit)
    Q_PROPERTY(QString datumAct READ liefereDatum)

public:
    explicit qtUhr(QObject *parent = nullptr);
    QString liefereZeit();
    QString liefereDatum();

private:
    QString zeitAct;
    QString datumAct;
};

#endif // QTUHR_H
