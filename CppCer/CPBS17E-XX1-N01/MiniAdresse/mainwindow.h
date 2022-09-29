/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.h
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
//für die SQL-Klassen
//außerdem muss in der Projektdatei der Eintrag sql
//hinzugefügt werden
#include <QtSql>
namespace Ui {
    class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //stellt die Verbindung zur Datenbank her
    bool dbVerbinden();
    QString iconNamen =  ":/Bild2.png";

private:
    Ui::MainWindow *ui;
    //steht die Verbindung zur Datenbank?
    bool dbVerbunden;
    //für die Datenbank
    QSqlDatabase datenbank;

private slots:
    //die Slots
    //sie nutzen den AutoConnect-Mechanismus
    void on_buttonListenAnzeige_clicked();
    void on_buttonEinzelAnzeige_clicked();
    void on_buttonNeuerEintrag_clicked();
    void on_buttonBeenden_clicked();
    //Aufgabe 2
    //Deklaration vom Slot
    void on_buttonSuchen_clicked();
};
#endif // MAINWINDOW_H
