import QtQuick 2.7
import QtQuick.Layouts 1.3

Rectangle {
    id: canvas

    radius: 10
    color: "#555555"
    border.width: 4
    border.color: "gray"
    width: childrenRect.width //socketRadius * 2 + 40
    height: childrenRect.height

    // User-facing properties
    property int inputs
    property int outputs
    property string title

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

        Loader {
            id: inputSockets

            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 15
            Layout.bottomMargin: 15
            anchors.horizontalCenter: parent.left

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = inputs
            }
        }

        Text {
            Layout.topMargin: 5
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            text: canvas.title
        }

        Loader {
            id: outputSockets

            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: 15
            Layout.bottomMargin: 15
            anchors.horizontalCenter: parent.right

            sourceComponent: socketColumn
            onLoaded: {
                item.sockets = outputs
            }
        }
    }
}
