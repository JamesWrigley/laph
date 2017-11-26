import QtQuick 2.7
import QtQuick.Layouts 1.3

Rectangle {
    id: root

    radius: 10
    color: "#555555"
    border.width: 4
    border.color: "gray"
    width: childrenRect.width
    height: childrenRect.height

    // User-facing properties
    property var inputs
    property var outputs
    property string title
    property Component ui

    Component {
        id: socketColumn

        Column {
            id: column

            spacing: 10

            property var sockets
            property bool floatRight

            Repeater {
                model: parent.sockets

                RowLayout {
                    LayoutMirroring.enabled: !floatRight

                    Rectangle {
                        id: socket

                        width: 14
                        height: width
                        radius: width / 2
                        border.width: 1
                        border.color: Qt.darker(color, 2)
                        color: "purple"
                    }

                    Text {
                        id: label

                        width: text.width
                        text: modelData
                    }
                }
            }
        }
    }

    RowLayout {
        anchors.top: parent.top

        property real margin: 15

        Loader {
            id: inputSockets

            Layout.alignment: Qt.AlignTop
            Layout.topMargin: parent.margin
            Layout.bottomMargin: parent.margin
            Layout.leftMargin: -5

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = inputs
                item.floatRight = true
            }
        }

        ColumnLayout {
            Layout.topMargin: parent.margin / 2
            Layout.bottomMargin: parent.margin
            Layout.alignment: Qt.AlignTop

            width: childrenRect.width

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: root.title
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 4

                width: parent.width * 0.75
                height: 1
                color: "black"
            }

            Loader { sourceComponent: root.ui }
        }

        Loader {
            id: outputSockets

            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: parent.margin
            Layout.bottomMargin: parent.margin
            Layout.rightMargin: -5

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = outputs
                item.floatRight = false
            }
        }
    }
}
