/*******************************************************
* Die Datei memorykarte.h
*******************************************************/
#ifndef MEMORYKARTE_H
#define MEMORYKARTE_H
#include <QTableWidgetItem>
#include <QIcon>
#include <QString>

//die Klasse für eine Karte des Memory-Spiels
//Sie erbt von QTableWidgetItem
class memorykarte : public QTableWidgetItem
{
    //die Attribute
    //eine eindeutige Identifizierung des Bildes
    int bildID;
    //die Icons für die Vorder- und die Rückseite
    QIcon bildVorne, bildHinten;
    //die Position im Spielfeld
    int bildPos;
    //ist die Karte umgedreht?
    bool umgedreht;
    //ist die Karte noch im Spiel?
    bool nochImSpiel;

    //die Methoden
    public:
    //der Konstruktor
    //er erhält den Namen für die Bilddatei der Vorderseite
    //und die ID des Bildes
    memorykarte(QString vorne, int id);
    //zum Anzeigen der jeweils anderen Kartenseite
    void umdrehen();
    //die Karte aus dem Spiel nehmen
    void rausnehmen();
    //einige Methoden zum Zugriff auf die Werte
    int getBildID();
    int getBildPos();
    void setBildPos(int position);
    bool getUmgedreht();
    bool getNochImSpiel();
};
#endif
