/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.h
* Heft: 16E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//für die Datei
#include <QFile>
//für die Standarddialoge
#include <QFileDialog>
//für die MessageBox
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString iconNamen =  ":/symbol/Bild2.png";
    //Aufgabe 2
    //Attribut namens dateiName wird deklariert.
    QString dateiName;

private:
    Ui::MainWindow *ui;

private slots:
    //die Slots
    void dateiOeffnen();
    void dateiSpeichern();
    void dateiSpeichernUnter(QString);
    void dateiNeu();

    void on_actionInfo_triggered();
    void on_actionSpeichern_unter_triggered();
};

#endif // MAINWINDOW_H
