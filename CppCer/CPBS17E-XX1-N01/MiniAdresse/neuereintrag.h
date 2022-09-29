/*******************************************************************
* Einsendeaufgabe
* Datei: neuereintrag.h
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef NEUEREINTRAG_H
#define NEUEREINTRAG_H
#include <QWidget>
//für die SQL-Klassen
#include <QtSql>

namespace Ui {
    class neuerEintrag;
}
class neuerEintrag : public QWidget
{
    Q_OBJECT

public:
    explicit neuerEintrag(QWidget *parent = 0);
    ~neuerEintrag();
    QString iconNamenForm =  ":/Bild1.png";

private slots:
    //die Slots
    //sie nutzen den AutoConnect-Mechanismus
    void on_buttonOK_clicked();
    void on_buttonBeenden_clicked();

private:
    Ui::neuerEintrag *ui;
    //für das Modell
    QSqlTableModel *modell;
};
#endif // NEUEREINTRAG_H
