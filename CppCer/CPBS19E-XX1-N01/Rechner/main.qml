/*******************************************************************
* Einsendeaufgabe
* Datei: main.qml
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Rechner")

    Page1 {
        //das übergeordnete Element ausfüllen
        anchors.fill: parent
    }

}
