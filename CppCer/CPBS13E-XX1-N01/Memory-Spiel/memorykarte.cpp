/*******************************************************
* Die Datei memorykarte.cpp
*******************************************************/
#include "memorykarte.h"
//der Konstruktor
memorykarte::memorykarte(QString vorne, int id)
{
    //die Vorderseite, der Dateiname des Bildes wird an den
    //Konstruktor übergeben
    bildVorne = QIcon(vorne);
    //die Rückseite ist festgesetzt
    bildHinten = QIcon(":/bilder/back.bmp");
    //die Rückseite wird angezeigt
    setIcon(bildHinten);
    //die Karte ist nicht umgedreht
    umgedreht = false;
    //die Karte ist noch im Spiel
    nochImSpiel = true;
    //die Nummer setzen
    bildID = id;
}

void memorykarte::umdrehen()
{
    //ist die Karte schon umgedreht?
    //dann die Rückseite anzeigen, aber nur dann, wenn die
    //Karte noch im Spiel ist
    if (nochImSpiel == true)
    {
        if (umgedreht == true)
        {
            setIcon(bildHinten);
            umgedreht = false;
        }
        //sonst die Vorderseite zeigen
        else
        {
            setIcon(bildVorne);
            umgedreht = true;
        }
    }
}

//die Karte aus dem Spiel nehmen
void memorykarte::rausnehmen()
{
    //die Grafik für aufgedeckt zeigen
    setIcon(QIcon(":/bilder/aufgedeckt.bmp"));
    nochImSpiel = false;
}

//die Bild-ID liefern
int memorykarte::getBildID()
{
    return bildID;
}

//die Position liefern
int memorykarte::getBildPos()
{
    return bildPos;
}

//die Position setzen
void memorykarte::setBildPos(int position)
{
    bildPos = position;
}

//den Status von nochImSpiel liefern
bool memorykarte::getNochImSpiel()
{
    return nochImSpiel;
}

//den Status von umgedreht liefern
bool memorykarte::getUmgedreht()
{
    return umgedreht;
}
