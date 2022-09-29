/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.h
* Heft: 18E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString iconNamen =  ":/icon/Bild2.png";
    QString iconNamenForm =  ":/icon/Bild1.png";

private:
    Ui::MainWindow *ui;

    //für die Liste
    QList<int> liste1;
    QList<int> liste1New;

    //eine Methode zum Aktualisieren der Liste
    void listeAktualisieren();

    //die Slots
    //Sie nutzen den AutoConnect-Mechanismus
private slots:
    void on_buttonNeu_clicked();
    void on_buttonNeuVor_clicked();
    void on_buttonLoeschen_clicked();
    void on_buttonAlleLoeschen_clicked();
    void on_buttonSortieren_clicked();
    void on_buttonSuchen_clicked();
    void on_buttonBeenden_clicked();
    //Aufgabe 4
    //Deklaration von den Slots
    void on_buttonRemove_clicked();
    void on_buttonInput_clicked();
};

#endif // MAINWINDOW_H
