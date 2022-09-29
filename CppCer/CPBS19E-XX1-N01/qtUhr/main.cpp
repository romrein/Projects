/*******************************************************************
* Einsendeaufgabe
* Datei: main.cpp
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "qtuhr.h"
#include <QIcon>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    //die Registrierung der Qt-Klasse
    qmlRegisterType<qtUhr>("Uhr", 1, 0, "QtUhr");
    app.setWindowIcon(QIcon(":/Bild2.png"));

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
