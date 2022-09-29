import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {
    id: item1
    property alias gridLayout: gridLayout

    property alias textField: textField
    property alias textField1: textField1
    property alias ausgabe: label
    property alias button11: button11
    property alias button15: button15
    property alias hinweisDialogKonvertierung: hinweisDialogKonvertierung
    property alias hinweisDialogDivision: hinweisDialogDivision

    GridLayout {
        id: gridLayout
        anchors.fill: parent
        rows: 8
        columns: 4

        TextField {
            id: textField
            text: qsTr("")
            font.pointSize: 14
            placeholderText: "Zahl 1"
            focus: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 4
        }

        TextField {
            id: textField1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 4
            text: qsTr("")
            font.pointSize: 14
            placeholderText: "Zahl 2"
        }

        Label {
            id: label
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 4
            text: qsTr("Ergebnis")
            font.pointSize: 14
        }

        Button {
            id: button10
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("+")
            font.pointSize: 20
        }

        Button {
            id: button2
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("7")
            font.pointSize: 20
        }

        Button {
            id: button1
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("8")
            font.pointSize: 20
        }

        Button {
            id: button
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("9")
            font.pointSize: 20
        }

        Button {
            id: button13
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("*")
            font.pointSize: 20
        }

        Button {
            id: button5
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("4")
            font.pointSize: 20
        }

        Button {
            id: button4
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("5")
            font.pointSize: 20
        }

        Button {
            id: button3
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("6")
            font.pointSize: 20
        }

        Button {
            id: button12
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("-")
            font.pointSize: 20
        }

        Button {
            id: button8
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("1")
            font.pointSize: 20
        }

        Button {
            id: button7
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("2")
            font.pointSize: 20
        }

        Button {
            id: button6
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("3")
            font.pointSize: 20
        }

        Button {
            id: button14
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("/")
            font.pointSize: 20
        }

        Button {
            id: button11
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr(",")
            font.pointSize: 20
        }
        Button {
            id: button9
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("0")
            Layout.columnSpan: 2
            font.pointSize: 20
        }

        Button {
            id: button15
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: qsTr("C")
            Layout.columnSpan: 4
            Layout.rowSpan: 1
            font.pointSize: 20
        }
    }
    MessageDialog {
        id: hinweisDialogKonvertierung
        title: "Bitte beachten"
        text: "Die Konvertierung ist gescheitert."
    }

    MessageDialog {
        id: hinweisDialogDivision
        title: "Bitte beachten"
        text: "Eine Division durch 0 ist nicht definiert."
    }

    Connections {
        target: button
        onClicked: rechnen.ziffernEingabe(button.text)
    }

    Connections {
        target: button1
        onClicked: rechnen.ziffernEingabe(button1.text)
    }

    Connections {
        target: button2
        onClicked: rechnen.ziffernEingabe(button2.text)
    }

    Connections {
        target: button3
        onClicked: rechnen.ziffernEingabe(button3.text)
    }

    Connections {
        target: button4
        onClicked: rechnen.ziffernEingabe(button4.text)
    }

    Connections {
        target: button5
        onClicked: rechnen.ziffernEingabe(button5.text)
    }

    Connections {
        target: button6
        onClicked: rechnen.ziffernEingabe(button6.text)
    }

    Connections {
        target: button7
        onClicked: rechnen.ziffernEingabe(button7.text)
    }

    Connections {
        target: button8
        onClicked: rechnen.ziffernEingabe(button8.text)
    }

    Connections {
        target: button9
        onClicked: rechnen.ziffernEingabe(button9.text)
    }

    Connections {
        target: button10
        onClicked: rechnen.operatorEingabe(button10.text)
    }

    Connections {
        target: button12
        onClicked: rechnen.operatorEingabe(button12.text)
    }

    Connections {
        target: button13
        onClicked: rechnen.operatorEingabe(button13.text)
    }

    Connections {
        target: button14
        onClicked: rechnen.operatorEingabe(button14.text)
    }
}
