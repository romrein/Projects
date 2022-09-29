/*******************************************************************
* Einsendeaufgabe
* Datei: Page1.qml
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
import QtQuick 2.7
import Uhr 1.0

Page1Form {
    //Aufgabe 3: Defininition der Eigenschaft "datumV"
    property bool datumV: false

    //Aufgabe 3: Nach dem vollständigen Laden einer Komponente wird das
    //Anzeigen der aktuellen Uhrzeit gestartet.
    Component.onCompleted:
    {
        meinTimer.start();
        aktualisiereZeit()
    }
    /*********************************************************************
    * mouseArea.onClicked: {}
    *
    * Aufgabe 3
    * Funktion: Beim ersten Anklicken wird ein Label mit der Uhrzeit
    * ausgeblendet, ein Timer für 2 Sekunden gestartet und in der Zeit ein
    * zweites Label mit aktuellem Datum versorgt und angezeigt. Zusätzlich,
    * kann beim zweiten Anklicken wieder die Uhrzeit erneut eingeblendet
    * werden.
    *********************************************************************/
    mouseArea.onClicked: {
        if (datumV == false)
        {
            datumV = true;
            label1.visible = true;
            label.visible = false;
            label1.text = meineQUhr.datumAct;
            meinTimerDatum.start();
        }
        else
        {
            datumV = false;
            label1.visible = false;
            label.visible = true;
            meinTimerDatum.stop();
        }
    }

    /*********************************************************************
    * Timer {}
    *
    * Aufgabe 3
    * Funktion: Konfiguration von dem Timer, der in einem Zeitintervall von
    * 100[ms] wiederholend die Uhrzeit anzeigt.
    *********************************************************************/
    Timer {
        id: meinTimer
        //er soll alle 100 Millisekunden ausgelöst werden
        interval: 100
        //wiederholt werden
        repeat: true
        //und die Funktion aktualisiereZeit() aufrufen
        onTriggered: aktualisiereZeit()
    }

    /*********************************************************************
    * Timer {}
    *
    * Aufgabe 3
    * Funktion: Konfiguration von dem Timer, der in einem Zeitintervall von
    * 2[s] die Uhrzeit ausblendet und die Anzeige vom aktuellen
    * Datum ermöglicht.
    *********************************************************************/
    Timer {
        id: meinTimerDatum
        //er soll 2000 Millisekunden lang das Datum anzeigen
        interval: 2000
        //darf nicht wiederholt werden
        repeat: false
        //und am Ende die Funktion zuUhr() aufrufen
        onTriggered: zuUhr()
    }

    //das Objekt für die Uhr
    QtUhr {
        id: meineQUhr
    }

    //die Funktion für den Timer
    /*********************************************************************
    * function aktualisiereZeit()
    *
    * Aufgabe 3
    * Funktion: Die aktuelle Zeit wird vom Timer gesteuert angezeigt.
    *********************************************************************/
    function aktualisiereZeit() {       
        label.text = meineQUhr.zeitAct;
    }

    //die Funktion für den Timer
    /*********************************************************************
    * function zuUhr()
    *
    * Aufgabe 3
    * Funktion: Führt ein Wiederkehren zu der Uhrzeit durch mit Setzen bzw.
    * Rücksetzen von entsprechenden Eigenschaften.
    *********************************************************************/
    function zuUhr() {
        datumV = false;
        label1.visible = false;
        label.visible = true;
    }
}
