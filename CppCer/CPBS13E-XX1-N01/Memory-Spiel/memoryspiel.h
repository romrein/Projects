/*******************************************************************
* Einsendeaufgabe
* Datei: memoryspiel.h
* Heft: 13E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

#ifndef MEMORYSPIEL_H
#define MEMORYSPIEL_H
//die Header-Dateien
#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QSpinBox>
#include "memorykarte.h"

//unsere Klasse memoryspiel erbt von QWidget
class memoryspiel : public QWidget
{
    //das Makro Q_OBJECT
    Q_OBJECT
    QSpinBox *spielstaerkeSpinBox;

    //die Attribute
    //die Tabelle für das Spielfeld
    QTableWidget* spielfeld;

    QLabel *labelTextMensch, *labelMensch, *labelTextComputer, *labelComputer;
    //Aufgebe 2: Deklaration von labelSpieler und labelTextSpieler
    QLabel *labelSpieler, *labelTextSpieler, *labelTextSpielstaerke;

    QString bildNamen[21] = {
        ":/bilder/apfel.bmp", ":/bilder/birne.bmp",
        ":/bilder/blume.bmp", ":/bilder/blume2.bmp",
        ":/bilder/ente.bmp", ":/bilder/fisch.bmp",
        ":/bilder/fuchs.bmp", ":/bilder/igel.bmp",
        ":/bilder/kaenguruh.bmp", ":/bilder/katze.bmp",
        ":/bilder/kuh.bmp", ":/bilder/maus1.bmp",
        ":/bilder/maus2.bmp", ":/bilder/maus3.bmp",
        ":/bilder/melone.bmp", ":/bilder/pilz.bmp",
        ":/bilder/ronny.bmp", ":/bilder/schmetterling.bmp",
        ":/bilder/sonne.bmp", ":/bilder/wolke.bmp",
        ":/bilder/maus4.bmp"
    };
    QString iconNamen =  ":/pix/Bild2.png";

    memorykarte *karten[42];

    //Aufgabe 3: Deklaration der Schaltfläche
    QPushButton *schummelnButton;

    //für den Timer
    //Aufgebe 3: Deklaration von dem timerSchummeln
    QTimer *timerUmdrehen, *timerSchummeln;
    //für die offenen Karten
    int umgedrehteKarten;
    //für den Spieler
    int spieler;
    //Aufgebe 1: Deklaration von dem spielerText
    QString spielerText;
    //für das aktuell umgedrehte Paar
    memorykarte *paar[2];
    //für die Punkte
    //Aufgabe 1: Deklaration von dem spielerPunkte
    int menschPunkte, computerPunkte, spielerPunkte;
    //das "Gedächtnis" für den Computer
    int gemerkteKarten[2][21];
    int spielstaerke = 1;

    //die Methoden
    //der Konstruktor
public:
    memoryspiel();

private:
    void karteOeffnen(memorykarte *karte);
    void kartenSchliessen();
    void computerZug();
    void paarPruefen(int kartenID);
    void paarGefunden();
    void spielerWechseln();
    bool zugErlaubt();
    //Aufgabe 3: Deklaration von Schummeln-Methode
    void schummelnKarten();

//die Slots
private slots:
    void mausKlickSlot(int x, int y);
    void timerSlot();
    //Aufgabe 3: Deklaration von einem Slot für den Schummeln-Timer
    void timerSlotSchummeln();
    //Aufgabe 3: Deklaration von einem Slot für die Schummeln-Schaltfläche
    void schummelnSlot();

    //Deklaration von einem Slot für die Spielstärke
    void spielstaerkeAendern(int);

};
#endif
