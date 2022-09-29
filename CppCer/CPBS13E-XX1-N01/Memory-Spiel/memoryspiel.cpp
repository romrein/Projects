/*******************************************************************
* Einsendeaufgabe
* Datei: memoryspiel.cpp
* Heft: 13E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

//die Header-Datei der eigenen Klasse
#include "memoryspiel.h"
#include <QHeaderView>
#include <QMessageBox>
#include <algorithm>
#include <ctime>

//der Konstruktor
memoryspiel::memoryspiel()
{
    //die Größe setzen
    resize(394, 550);
    //und den Titel
    setWindowTitle("Memoryspiel");

    this->setWindowIcon(QIcon(iconNamen));

    //Aufgabe 3: Definition einer Schaltfläche für eine Schummeln-Methode
    schummelnButton = new QPushButton(this);
    schummelnButton->setText("Schummeln");
    schummelnButton->setGeometry(300, 460, 70, 25);

    //den Timer erstellen
    timerUmdrehen = new QTimer(this);
    timerUmdrehen->setSingleShot(true);

    //Aufgabe 3: Definition von einem Timer für die Schummeln-Methode
    timerSchummeln = new QTimer(this);
    timerSchummeln->setSingleShot(true);

    //die Attribute initialisieren
    //es ist keine Karte umgedreht
    umgedrehteKarten = 0;
    //der Spieler 0 – der Mensch – fängt an
    spieler = 0;
    //es hat noch keiner Punkte
    menschPunkte = 0;
    computerPunkte = 0;
    //gemerkteKarten wird mit -1 initialisiert
    for (int aussen = 0; aussen < 2; aussen++)
        for (int innen = 0; innen < 21; innen++)
            gemerkteKarten[aussen][innen] = -1;

    //eine neue Tabelle mit sieben Zeilen und sechs Spalten
    //erzeugen
    spielfeld = new QTableWidget(7, 6, this);

    //die Spalten- und Zeilenköpfe ausblenden
    spielfeld->horizontalHeader()->hide();
    spielfeld->verticalHeader()->hide();
    //das Gitternetz ausblenden
    spielfeld->setShowGrid(false);
    //die Auswahl deaktivieren
    spielfeld->setSelectionMode(QTableWidget::NoSelection);
    //das Bearbeiten deaktivieren
    spielfeld->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //die Höhe und Breite der Zellen setzen
    for (int zeile = 0; zeile < 7; zeile++)
        spielfeld->setRowHeight(zeile, 64);
    for (int spalte = 0; spalte < 6; spalte++)
        spielfeld->setColumnWidth(spalte, 64);

    //die Größe anpassen
    spielfeld->resize(394, 458);

    labelTextMensch = new QLabel(this);
    labelTextMensch->setText("Mensch");
    labelTextMensch->setGeometry(10, 460, 50, 25);

    labelMensch = new QLabel(this);
    labelMensch->setText("0");
    labelMensch->setGeometry(80, 460, 50, 25);

    labelTextComputer = new QLabel(this);
    labelTextComputer->setText("Computer");
    labelTextComputer->setGeometry(10, 480, 50, 25);

    labelComputer = new QLabel(this);
    labelComputer->setText("0");
    labelComputer->setGeometry(80, 480, 50, 25);

    //Aufgabe 2: Definition von einer Text-Label "Es zieht der"
    labelTextSpieler = new QLabel(this);
    labelTextSpieler->setText("Es zieht der");
    labelTextSpieler->setGeometry(10, 500, 60, 25);

    //Aufgabe 2: Definition von einer Text-Label und
    //Setzen die Anzeige "Mensch" als Bezeichnung für den Spieler,
    //der das Spiel beginnt.
    labelSpieler = new QLabel(this);
    labelSpieler->setText("Mensch");
    labelSpieler->setGeometry(80, 500, 50, 25);

    //Definition von einer Text-Label für die Spielstärke
    labelTextSpielstaerke = new QLabel(this);
    labelTextSpielstaerke->setText("Spielstärke:");
    labelTextSpielstaerke->setGeometry(10, 520, 60, 25);

    //Definition vom Drehfeld zum Verändern der Spielstärke
    QSpinBox *spielstaerkeSpinBox = new QSpinBox(this);
    spielstaerkeSpinBox->setRange(1, 100);
    spielstaerkeSpinBox->setToolTip("Spielstärke ändern");
    spielstaerkeSpinBox->setValue(0);
    spielstaerkeSpinBox->setGeometry(300, 520, 50, 25);

    int bildZaehler = 0;
    for (int schleife = 0; schleife < 42; schleife++)
    {
        //bitte in einer Zeile eingeben
        karten[schleife] = new memorykarte(bildNamen[bildZaehler], bildZaehler);
        if ((schleife + 1) % 2 == 0)
        bildZaehler++;
    }

    //die Karten durcheinander werfen
    srand(time(NULL));
    std::random_shuffle(karten, karten + 42);

    //die Größe der Icons einstellen
    spielfeld->setIconSize(QSize(64,64));
    //die Karten in die Tabelle stellen
    for (int zeile = 0; zeile < 7; zeile++)
        for (int spalte = 0; spalte < 6; spalte++)
        {
            //bitte jeweils in einer Zeile eingeben
            spielfeld->setItem(zeile, spalte, karten[(spalte * 7) + zeile]);
            //die Position setzen
            karten[(spalte * 7) + zeile]-> setBildPos((spalte * 7) + zeile);
        }

    //das Anklicken einer Zelle mit dem Slot mausKlickSlot
    //verbinden
    QObject::connect(spielfeld,SIGNAL(cellClicked(int, int)), this, SLOT(mausKlickSlot(int, int)));
    //den Timer mit dem Slot verbinden
    QObject::connect(timerUmdrehen,SIGNAL(timeout()), this, SLOT(timerSlot()));

    //Aufgabe 3: Signal-Slots Verbindungen für eine Schummeln-Methode
    QObject::connect(timerSchummeln,SIGNAL(timeout()), this, SLOT(timerSlotSchummeln()));
    QObject::connect(schummelnButton, SIGNAL(clicked()), this, SLOT(schummelnSlot()));

    //Signal-Slot Verbindung für die Änderung der Spielstärke
    QObject::connect(spielstaerkeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(spielstaerkeAendern(int)));
}

/*******************************************************************
* void memoryspiel::karteOeffnen(memorykarte *karte)
*
* Funktion: Karten oeffnen
********************************************************************/
void memoryspiel::karteOeffnen(memorykarte *karte)
{
    //zum Zwischenspeichern der Nummer und der Position
    int kartenID, kartenPos;
    //die Karten zwischenspeichern
    paar[umgedrehteKarten] = karte;
    //die Nummer und die Position beschaffen
    kartenID = karte->getBildID();
    kartenPos = karte->getBildPos();
    //die Karten in das Gedächtnis eintragen
    //aber nur dann, wenn es noch keinen Eintrag an der
    //entsprechenden Stelle gibt
    if (gemerkteKarten[0][kartenID] == -1)
        gemerkteKarten[0][kartenID] = kartenPos;
    else
        //wenn es schon einen Eintrag gibt und der nicht
        //mit der aktuellen Position übereinstimmt, haben
        //wir die zweite Karte gefunden
        //Sie wird in die zweite Dimension eingetragen
        if ((gemerkteKarten[0][kartenID] != kartenPos))
            gemerkteKarten[1][kartenID] = kartenPos;
        //umgedrehte Karten erhöhen
    umgedrehteKarten++;
    //sind zwei Karten umgedreht worden?
    //dann prüfen wir, ob es ein Paar ist
    if (umgedrehteKarten == 2)
    {
        paarPruefen(kartenID);
        //die Karten wieder umdrehen
        timerUmdrehen->start(2000);
    }
    //haben wir zusammen 21 Paare?
    //dann ist das Spiel vorbei
    if (menschPunkte + computerPunkte == 21)
    {
        timerUmdrehen->stop();

        //Aufgabe 3. Timer wird angehalten
        timerSchummeln->stop();

        //Aufgabe 1. Die Meldung beim Spielende wird um die Anzeige
        //des Gewinners und von ihm gezogene Karten-Paaren ergänzt.
        if (menschPunkte > computerPunkte)
        {
            spielerText = "Mensch";
            spielerPunkte = menschPunkte;
        }
        else
        {
            spielerText = "Computer";
            spielerPunkte = computerPunkte;
        }
        //Die Anzeige wird um zusätzliche Informationen wie die Bezeichnung des
        //Spielers und entsprechende Anzahl der gezogenen Karten-Paare erweitert.
        QString status = QString("%1 hat mit %2 Paaren gewonnen").arg(spielerText).arg(spielerPunkte);
        QMessageBox::information(this, "Spielende", status);

        this->close();
    }
}

/*******************************************************************
* void memoryspiel::paarPruefen(int kartenID)
*
* Funktion: Methode prüft, ob ein Paar gefunden wurde
********************************************************************/
void memoryspiel::paarPruefen(int kartenID)
{
    if (paar[0]->getBildID() == paar[1]->getBildID())
    {
        //die Punkte setzen
        paarGefunden();
        //die Karten aus dem Gedächtnis löschen
        gemerkteKarten[0][kartenID] = -2;
        gemerkteKarten[1][kartenID] = -2;
    }
}

/*******************************************************************
* void memoryspiel::paarGefunden()
*
* Funktion: Methode setzt die Punkte, wenn ein Paar gefunden wurde
********************************************************************/
void memoryspiel::paarGefunden()
{
    //spielt gerade der Mensch?
    if (spieler == 0)
    {
        menschPunkte++;
        labelMensch->setNum(menschPunkte);
    }
    else
    {
        computerPunkte++;
        labelComputer->setNum(computerPunkte);
    }
}

/*******************************************************************
* void memoryspiel::kartenSchliessen()
*
* Funktion: Methode dreht die Karten wieder um bzw. nimmt sie
* aus dem Spiel
********************************************************************/
void memoryspiel::kartenSchliessen()
{
    bool raus = false;
    //ist es ein Paar?
    if (paar[0]->getBildID() == paar[1]->getBildID())
    {
        //dann nehmen wir die Karten aus dem Spiel
        paar[0]->rausnehmen();
        paar[1]->rausnehmen();
        //raus wird auf true gesetzt
        raus = true;
    }
    else
    {
        //sonst drehen wir die Karten nur wieder um
        paar[0]->umdrehen();
        paar[1]->umdrehen();
    }
    //es ist keine Karte mehr geöffnet
    umgedrehteKarten = 0;
    //hat der Spieler kein Paar gefunden
    if (raus == false)
        //dann wird der Spieler gewechselt
        spielerWechseln();
    else
        //hat der Computer eine Paar gefunden?
        //dann ist er noch einmal an der Reihe
        if (spieler == 1)
            computerZug();
}

/*******************************************************************
* void memoryspiel::spielerWechseln()
*
* Funktion: Methode wechselt den Spieler
********************************************************************/
void memoryspiel::spielerWechseln()
{
    //wenn der Mensch an der Reihe war, kommt jetzt der Computer
    if (spieler == 0)
    {
        spieler = 1;
        //Aufgabe 2: Änderung von dem Text-Label auf die dem aktuellen
        //Spieler entsprechende Bezeichnung. In dem Fall "Computer"
        labelSpieler->setText("Computer");
        computerZug();
    }
    else
    {
        spieler = 0;
        //Aufgabe 2: Änderung von dem Text-Label auf die dem aktuellen
        //Spieler entsprechende Bezeichnung. In dem Fall "Mensch"
        labelSpieler->setText("Mensch");
    }
}

/*******************************************************************
* void memoryspiel::computerZug()
*
* Funktion: Methode für den Computerzug
********************************************************************/
void memoryspiel::computerZug()
{
    int kartenZaehler = 0;
    int zufall = 0;
    bool treffer = false;
    //zur Steuerung der Spielstärke
    if ((rand() % spielstaerke) == 0)
    {
        //erst einmal nach einem Paar suchen
        //dazu durchsuchen wir das Feld gemerkteKarten, bis wir
        //in den beiden Dimensionen einen Wert für eine Karte
        //finden
        while ((kartenZaehler < 21) && (treffer == false))
        {
            //gibt es in beiden Dimensionen einen Wert größer
            //oder gleich 0?
            //bitte in einer Zeile eingeben
            if ((gemerkteKarten[0][kartenZaehler] >= 0) && (gemerkteKarten[1][kartenZaehler] >= 0))
            {
                //dann haben wir ein Paar
                treffer = true;
                //die erste Karte umdrehen
                karten[gemerkteKarten[0][kartenZaehler]]-> umdrehen();
                karteOeffnen(karten[gemerkteKarten[0] [kartenZaehler]]);
                //und die zweite auch
                karten[gemerkteKarten[1][kartenZaehler]]-> umdrehen();
                karteOeffnen(karten[gemerkteKarten[1] [kartenZaehler]]);
            }
            kartenZaehler++;
        }
    }
    //wenn wir kein Paar gefunden haben, drehen wir
    //zufällig zwei Karten um
    if (treffer == false)
    {
        //so lange eine Zufallszahl suchen, bis eine Karte
        //gefunden wird, die noch im Spiel ist
        do {
            zufall = rand() % 42;
        } while (karten[zufall]->getNochImSpiel() == false);
        //die erste Karte umdrehen
        karten[zufall]->umdrehen();
        karteOeffnen(karten[zufall]);
        //für die zweite Karte müssen wir außerdem prüfen,
        //ob sie nicht gerade angezeigt wird
        do {
            zufall = rand() % 42;
            //bitte in einer Zeile eingeben
        } while ((karten[zufall]->getNochImSpiel() == false) || (karten[zufall]->getUmgedreht() == true));
        //und die zweite Karte umdrehen
        karten[zufall]->umdrehen();
        karteOeffnen(karten[zufall]);
    }
}

/*******************************************************************
* void memoryspiel::mausKlickSlot(int x, int y)
*
* Funktion: Slot für das Anklicken einer Zelle
********************************************************************/
void memoryspiel::mausKlickSlot(int x, int y)
{
    //darf der Mensch ziehen?
    if (zugErlaubt() == true)
    {
        //ist die Karte nicht schon umgedreht und ist sie noch
        //im Spiel?
        //bitte in einer Zeile eingeben
        if ((karten[(y * 7) + x]->getUmgedreht() == false) && (karten[(y * 7) + x]->getNochImSpiel() == true))
        {
            //dann umdrehen
            karten[(y * 7) + x]->umdrehen();
            karteOeffnen(karten[(y * 7) + x]);
        }
    }
}

/*******************************************************************
* void memoryspiel::timerSlot()
*
* Funktion: Slot für den Timer
********************************************************************/
void memoryspiel::timerSlot()
{
    //die Karten wieder umdrehen
    kartenSchliessen();
}

/*******************************************************************
* void memoryspiel::zugErlaubt()
*
* Funktion: die Methode prüft, ob Züge des Menschen erlaubt sind
* die Rückgabe ist false, wenn gerade der Computer zieht
* oder wenn schon zwei Karten umgedreht sind
* sonst ist die Rückgabe true
********************************************************************/
bool memoryspiel::zugErlaubt()
{
    bool erlaubt = true;
    //zieht der Computer?
    if (spieler == 1)
        erlaubt = false;
    //sind schon zwei Karten umgedreht?
    if (umgedrehteKarten == 2)
        erlaubt = false;
    return erlaubt;
}

/*******************************************************************
* void memoryspiel::timerSlotSchummeln()
*
* Aufgabe 3
* Funktion: Slot für den Schummeln-Timer. Wenn die vorgegebene Zeit
* abgelaufen ist, sollen die Karten wieder umgedreht werden.
********************************************************************/
void memoryspiel::timerSlotSchummeln()
{
    //Zeit ist abgelaufen, die Karten wieder umdrehen
    schummelnKarten();
}

/*******************************************************************
* void memoryspiel::schummelnSlot()
*
* Aufgabe 3
* Funktion: Slot für den Schaltfläche-Schummeln. Wenn die Schaltfläche
* "Schummeln" betätigt wurde, sollen die Karten angezeigt (geöffnet)
* und der Timer für spätere Umdrehen gestartet werden.
********************************************************************/
void memoryspiel::schummelnSlot()
{
    schummelnKarten();
    if (timerSchummeln->isActive())
    {
        timerSchummeln->stop();
    }
    else
    {
        timerSchummeln->start(2000);
    }
}

/*******************************************************************
* void memoryspiel::schummelnKarten()
*
* Aufgabe 3
* Funktion: Zwei verschachtelte FOR-Schleifen (siehe Heft 13E Seite 19)
* werden verwendet um die Karten umzudrehen. Die Prüfung bereits
* geöffneten und im Spiel vorhandenen Karten übernimmt die Methode
* karten[]->umdrehen() der Klasse memorykarte.
********************************************************************/
void memoryspiel::schummelnKarten()
{
    //die Karten wieder umdrehen
    for (int zeile = 0; zeile < 7; zeile++)
        for (int spalte = 0; spalte < 6; spalte++)
            karten[(spalte * 7) + zeile]->umdrehen();
}

/*******************************************************************
* void memoryspiel::spielstaerkeAendern(int neueSpielstarke)
* par[in]: int neueSpielstarke
*
* Funktion: Änderung der Spielstärke. Vorgegeben durch SpinBox
********************************************************************/
void memoryspiel::spielstaerkeAendern(int neueSpielstarke)
{
    spielstaerke = neueSpielstarke;
}
