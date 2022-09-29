/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.h
* Heft: 12E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//für den Medienplayer
#include <QMediaPlayer>
//für das Video-Widget
#include <QVideoWidget>
#include <QSlider>
#include <QSpinBox>
#include <QListWidgetItem>
#include <QLabel>
#include <QTime>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QMediaPlayer* meinPlayer;
    QSlider *lautstaerkeSlider, *positionsSlider;
    QSpinBox *lautstaerkeSpinBox;
    QLabel *positionsLabel;
    QString gesamtZeit;
    int iLautTmp;               //Aufgabe 2. Temporale Variable für aktuelle Lautstärke.

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

public slots:
    void listeUndTitelAktualisieren();
    void lautstaerkeAendern(int);
    void kontextmenuErzeugen();
    void positionAendern(int);
    void dauerGeaendert(qint64);
    void positionGeaendert(qint64);

private slots:
    void on_action_Beenden_triggered();
    void on_action_ffnen_triggered();
    void on_action_Pause_triggered();
    void on_actionW_iedergeben_triggered();
    void on_action_Stopp_triggered();
    void on_actionTon_ein_aus_triggered();
    void on_action_Lautst_rke_triggered();
    void on_actionW_iedergabe_triggered();
    void on_action_Wiedergabeliste_triggered();
    void on_listWidget_itemClicked(QListWidgetItem* );
    void on_actionEintrag_l_schen_triggered();
    void on_actionEintr_ge_sortieren_triggered();
    void on_actionInfo_triggered();
    void on_action_Vollbild_triggered();
};
#endif // MAINWINDOW_H
