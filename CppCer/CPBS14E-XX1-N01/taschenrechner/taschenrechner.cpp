/*******************************************************************
* Einsendeaufgabe
* Datei: taschenrechner.cpp
* Heft: 14E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

//die Header-Dateien einbinden
#include "taschenrechner.h"
#include "ui_taschenrechner.h"
#include <QMessageBox>

//Aufgabe 4.
enum enSelect {enInfinity, enError, enKorrekt};

Taschenrechner::Taschenrechner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Taschenrechner)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(iconNamen));
}

Taschenrechner::~Taschenrechner()
{
    delete ui;
}

//der Slot für die Schaltfläche
void Taschenrechner::on_pushButton_clicked()
{
    float zahl1, zahl2, ergebnis = 0;
    bool flag = false;
    //für die Ausgabe
    QString ausgabe;

    /**************************************************************************************
    * Aufgabe 4. Aufruf der Konvertierungsmethode und die Übergabe von Parametern.
    **************************************************************************************/
    if( (!Konvertierung(ui->lineEdit, zahl1)) && (!Konvertierung(ui->lineEdit_2, zahl2)) )
    {
        //welche Rechenoperation ist ausgewählt?
        //die Addition
        if (ui->radioButtonAddition->isChecked() == true)
            ergebnis = zahl1 + zahl2;
        //die Subtraktion
        if (ui->radioButtonSubtraktion->isChecked() == true)
            ergebnis = zahl1 - zahl2;
        //die Division
        if (ui->radioButtonDivision->isChecked() == true)
        {
            //ist die zweite Zahl ungleich 0?
            if (zahl2 != 0)
                ergebnis = zahl1 / zahl2;
            else
                flag = true;
        }
        //die Multiplikation
        if (ui->radioButtonMultiplikation->isChecked() == true)
            ergebnis = zahl1 * zahl2;

        //die Ausgabe setzen
        //wurde eine Division durch 0 versucht?
        if (flag == true)
            ausgabe = "Nicht definiert!";
        else
            ausgabe = locale().toString(ergebnis);

        //das Ergebnis anzeigen
        ui->label->setText(ausgabe);
        //und auch in der LCD-Anzeige
        ui->lcdNumber->display(ergebnis);
    }
    else
    {
        //Aufgabe 4:
        //Fehler bei der Konvertierung aufgetretten, im Label anzeigen.
        ui->label->setText("ERROR");
    }
}

/*******************************************************************
* int Taschenrechner::Konvertierung()
*
* Aufgabe 4
* Parameter [In]:   QLineEdit   *LEdit
*                   float       &zahl
* Parameter [Out]:  int         err
* Funktion: Ausnahmebehandlung für die Konvertierungsfehler bei den
* beiden Eingabefeldern.
********************************************************************/

int Taschenrechner::Konvertierung(QLineEdit *LEdit, float &zahl)
{
    int err = 0;
    bool ok = false;
    QString status;
    try
    {
        zahl = locale().toFloat(LEdit->text(), &ok);
        //Der Fall für die Unendlichkeit wird geprüft
        if (qIsInf(zahl))
        { throw 0; }
        //Ungültige Eingabe
        else if (ok == false)
        { throw 1; }
        //Korrekte Eingabe
        else
        { throw 2; }
    }
    catch (int e)
    {
        switch(e)
        {
            case enInfinity:
                status = QString("Eingabe ist (+-)unendlich").arg(LEdit->text());
                QMessageBox::information(this, "Hinweis", status);
                LEdit->setFocus();
                //An der Stelle kann ein (-1) an die "Berechnung" übergeben werden, um diese
                //zu überspringen und ein "ERROR" in dem Label anzuzeigen. Allerdings, kann die
                //Methode ->toString() auch selbst solche Fehler identifizieren und dementsprechend
                //"inf", "-inf" oder "nan" im Label setzten.
                err = 0;
                //sonst err = -1;
            break;
            case enError:
                //Fehlerfall. Ungültige Eingabe.
                status = QString("Eingabe [ %1 ] ist nicht korrekt").arg(LEdit->text());
                QMessageBox::information(this, "Hinweis", status);
                LEdit->setFocus();
                err = -1;
            break;
            case enKorrekt:
                //Theoretisch unnötig, da bei der Definition bereits mit 0 initialisiert.
                err = 0;
            break;
            default:
                //Theoretisch auch unnötig, da "fast" ausgeschloßen, zur Sicherheit implementiert.
                QMessageBox::information(this, "Hinweis", "Unbekannter Fehler");
                LEdit->setFocus();
                err = -1;
            break;
        }
    }
    return err;
}

