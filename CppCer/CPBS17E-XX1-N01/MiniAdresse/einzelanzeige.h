/*******************************************************************
* Einsendeaufgabe
* Datei: einzelanzeige.h
* Heft: 17E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#ifndef EINZELANZEIGE_H
#define EINZELANZEIGE_H
#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QSqlTableModel>
#include <QLabel>

namespace Ui {
    class einzelAnzeige;
}
class einzelAnzeige : public QMainWindow
{
    Q_OBJECT
    QLabel *datenSatzLbl;
    QString iconNamenForm =  ":/Bild1.png";

public:
    explicit einzelAnzeige(QWidget *parent = 0);
    ~einzelAnzeige();
    void verbindungenHerstellen();
    void datenMappen();
    void indexRefresh();


private slots:
    //die Slots
    void ganzNachVorne();
    void einenNachVorne();
    void ganzNachHinten();
    void einenNachHinten();
    void loeschen();

private:
    Ui::einzelAnzeige *ui;
    //für den Mapper
    QDataWidgetMapper *mapper;
    //für das Modell
    QSqlTableModel *modell;
};
#endif // EINZELANZEIGE_H
