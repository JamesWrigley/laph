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
    property int inputs
    property int outputs
    property string title
    property Component ui

    // Private properties (do not modify)
    property real socketRadius: 14
    property real socketSpacing: 10

    function computeHeight() {
        return ( socketRadius * (inputs + outputs) +
                 (inputs + outputs - 2) * socketSpacing +
                 40 )
    }

    Component {
        id: socketColumn

        Column {
            spacing: socketSpacing

            property int sockets

            Repeater {
                model: parent.sockets

                Rectangle {
                    width: socketRadius
                    height: width
                    radius: width / 2
                    border.width: 1
                    border.color: Qt.darker(color, 2)
                    color: "purple"
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
            anchors.horizontalCenter: parent.left

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = inputs
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
            anchors.horizontalCenter: parent.right

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = outputs
            }
        }
    }
}
