/*******************************************************************
* Einsendeaufgabe
* Datei: taschenrechner.h
* Heft: 14E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef TASCHENRECHNER_H
#define TASCHENRECHNER_H

#include <QMainWindow>
#include <QLineEdit>
#include <QString>

namespace Ui {
class Taschenrechner;
}

class Taschenrechner : public QMainWindow
{
    Q_OBJECT

public:
    explicit Taschenrechner(QWidget *parent = 0);
    ~Taschenrechner();
    //Aufgabe 4. Deklaration der Methode zur Konvertierung
    int Konvertierung(QLineEdit *LEdit, float &zahl);
    QString iconNamen =  ":/bild/Bild2.png";


private slots:
    //für die Schaltfläche
    void on_pushButton_clicked();

private:
    Ui::Taschenrechner *ui;

};

#endif // TASCHENRECHNER_H
