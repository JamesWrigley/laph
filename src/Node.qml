import QtQuick 2.7
import QtQuick.Layouts 1.3

Rectangle {
    radius: 10
    color: "#555555"
    border.width: 4
    border.color: "gray"
    width: socketRadius * 2 + 40
    height: computeWidth()

    // User-facing properties
    property int inputs
    property int outputs
    property string title

    // Private properties (do not modify)
    property real socketRadius: 14
    property real socketSpacing: 10

    function computeWidth() {
        return ( socketRadius * (inputs + outputs) +
                 (inputs + outputs - 2) * socketSpacing +
                 40 )
    }

    Component {
        id: socketColumn

        Column {
            anchors.topMargin: 15
            anchors.bottomMargin: anchors.topMargin

            spacing: socketSpacing

            property int sockets

            Repeater {
                model: parent.sockets

                Rectangle {
                    width: socketRadius
                    height: width
                    radius: width / 2
                    border.width: 1
                    border.color: "pink"
                    color: "white"
                }
            }
        }
    }

    Loader {
        id: inputSockets

        anchors.top: parent.top
        anchors.horizontalCenter: parent.left

        sourceComponent: socketColumn
        onLoaded: {
            item.sockets = inputs
            item.anchors.top = top
        }
    }

    Loader {
        id: outputSockets

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.right

        sourceComponent: socketColumn
        onLoaded: {
            item.sockets = outputs
            item.anchors.bottom = bottom
       }
    }
}
