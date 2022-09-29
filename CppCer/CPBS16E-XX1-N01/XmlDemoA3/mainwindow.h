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
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString iconNamen =  ":/Bild/Bild2.png";

private:
    Ui::MainWindow *ui;
    //für den Namen der Datei
    QString xmlName;
    //ist die Datei vorhanden?
    bool xmlVorhanden;
    //für die Datei
    QFile *datei;

    //Aufgabe 3
    //Funktion: Deklaration von den Slots
private slots:
    void on_buttonSchreiben_clicked();
    void on_pushButtonList_clicked();
};

#endif // MAINWINDOW_H
