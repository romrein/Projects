/*******************************************************************
* Einsendeaufgabe
* Datei: Page1.qml
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
import QtQuick 2.7

Page1Form {
    //die ID setzen
    id: rechnen;
    //die Eigenschaft für das aktive Feld
    property int aktiveEingabe: 1;

    //für das Komma
    button11.onClicked: {
        //Aufgabe 2
        //Es wird dafür gesorgt, dass nur maximal ein Komma für
        //jede Zahl eingegeben werden kann. Die Funktion "indexOf"
        //gibt die Position des  Zeichens in der Zeichenkette
        //zurück oder -1 wenn kein Zeichen gefunden wurde.
        if (aktiveEingabe == 1)
        {
            if (textField.text.indexOf(",") === -1)
            {
                textField.text = textField.text + ",";
            }
            else
            {
                //Das Komma ist bereits vorhanden
            }
        }
        else
        {
            if (textField1.text.indexOf(",") === -1)
            {
                textField1.text = textField1.text + ",";
            }
            else
            {
                //Das Komma ist bereits vorhanden
            }
        }
    }

    //welches Feld ist aktiv?
    textField.onActiveFocusChanged: {
        //hat das Feld den Fokus?
        if (textField.focus == true)
            aktiveEingabe = 1;
    }

    textField1.onActiveFocusChanged: {
        //hat das Feld den Fokus?
        if (textField1.focus == true)
            aktiveEingabe = 2;
    }

    button15.onClicked: {
        //alles wieder zurücksetzen
        textField.text = "";
        textField1.text = "";
        ausgabe.text = "";
    }

    //die Funktion für die Eingabe
    function ziffernEingabe(ziffer) {
        //in welches Feld soll die Eingabe gestellt werden?
        if (aktiveEingabe == 1)
            textField.text = textField.text + ziffer;
        else
            textField1.text = textField1.text + ziffer;
    }

    //die Methode für das Anklicken der Operatorschaltflächen
    function operatorEingabe(operator) {
        //für das Ergebnis
        var ergebnis = 0.0

        //Aufgabe 1
        //Das Komma vor der Rechnung durch einen Punkt austauschen
        textField.text = textField.text.replace(",", ".");
        textField1.text = textField1.text.replace(",", ".");

        //die Werte umwandeln
        var zahl1 = parseFloat(textField.text);
        var zahl2 = parseFloat(textField1.text);
        //hat die Umwandlung geklappt?
        if (isNaN(zahl1) || isNaN(zahl2)) {
            hinweisDialogKonvertierung.open();
            return;
        }
        //und auswerten
        switch (operator) {
        case "+":
            ergebnis = zahl1 + zahl2;
            break;
        case "-":
            ergebnis = zahl1 - zahl2;
            break;
        case "*":
            ergebnis = zahl1 * zahl2;
            break;
        case "/":
            //wird eine Division durch 0 versucht?
            if (zahl2 === 0) {
                hinweisDialogDivision.open();
                return;
            }
            ergebnis = zahl1 / zahl2;
            break;
        default:
            console.log("Ein Fehler ist aufgetreten");
        }
        //das Ergebnis im Label ausgeben
        //Aufgabe 1
        //Die Punkte nach der Rechnung durch Kommas wieder ersetzen
        textField.text = textField.text.replace(".", ",");
        textField1.text = textField1.text.replace(".", ",");
        ausgabe.text = ergebnis;
        ausgabe.text = ausgabe.text.replace(".", ",");
    }
}
