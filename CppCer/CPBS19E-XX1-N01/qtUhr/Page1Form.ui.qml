/*******************************************************************
* Einsendeaufgabe
* Datei: Page1Form.ui.qml
* Heft: 19E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/
import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    //Aufgabe 3
    //Funktion: Definition von den Eigenschaften
    property alias label: label
    property alias mouseArea: mouseArea
    property alias label1: label1
    property alias rowLayout: rowLayout

    RowLayout {
        id: rowLayout
        anchors.fill: parent

        Label {
            id: label
            text: qsTr("00:00:00:00")
            font.pointSize: 35
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    /*********************************************************************
    * MouseArea {}
    *
    * Aufgabe 3
    * Funktion: Ein Steuerelement vom Typ MouseArea wird definiert und das
    * Label in diesen Steuerelement verschachtelt abgelegt.
    *********************************************************************/
    MouseArea {
        id: mouseArea
        anchors.fill: parent

        Label {
            id: label1
            visible: false
            text: qsTr("Datum")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.rightMargin: 0
            font.pointSize: 35
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4}D{i:3}
}
##^##*/
