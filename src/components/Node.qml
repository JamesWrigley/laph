import QtQuick 2.7
import QtQuick.Layouts 1.3

import "."

Rectangle {
    id: root

    radius: 10
    color: "#555555"
    border.width: 4
    border.color: "gray"
    width: childrenRect.width
    height: childrenRect.height
    x: xDrag + xOffset
    y: yDrag + yOffset

    // User-facing properties
    property var inputs
    property var outputs
    property string title
    property Component ui

    // Private properties (subclassing nodes should not touch these)
    property int index
    property var canvas
    property real xDrag
    property real yDrag
    property real xOffset
    property real yOffset

    FontLoader {
        id: sans
        source: "../fonts/FiraSans-Regular.otf"
    }

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
                        color: modelData[1] == Socket.scalar ? "purple" : "green"

                        property alias onLeft: da.onLeft

                        DropArea {
                            id: da
                            anchors.fill: parent

                            property var node: root
                            property bool onLeft: !floatRight
                            property int wires: children.length
                            property var socketType: modelData[1]

                            onDropped: {
                                if (drop.source.twinIndex != root.index &&
                                    drop.source.twinSide == !onLeft) {
                                    drop.accept(Qt.MoveAction)
                                }
                            }
                        }

                        MouseArea {
                            id: ma

                            width: parent.width
                            height: parent.height

                            drag.target: wire == null ? undefined : wire.endTip
                            drag.threshold: 0
                            drag.axis: Drag.XAndYAxis
                            enabled: wire == null && da.wires == 0

                            property var wire: null

                            onPressed: {
                                var component = Qt.createComponent("../core/Wire.qml")
                                if (component.status == Component.Ready) {
                                    wire = component.createObject(da, {"dragging": true,
                                                                       "startIndex": root.index,
                                                                       "startOnLeft": parent.onLeft,
                                                                       "canvas": Qt.binding(function () { return canvas }),
                                                                       "startUpdateHook": Qt.binding(function () {
                                                                           return root.x + root.y
                                                                       }),
                                                                       "endUpdateHook": Qt.binding(function () {
                                                                           return ma.mouseX + ma.mouseY
                                                                       })})

                                    if (wire == null) {
                                        console.error("Object 'Wire.qml' could not be created")
                                    }
                                } else {
                                    console.error("Component 'Wire.qml' is not ready:", component.status)
                                }
                            }

                            onReleased: {
                                wire.handleRelease(wire.endTip)
                            }
                        }
                    }

                    Text {
                        id: label

                        width: text.width
                        text: modelData[0]
                        color: "#202020"
                        font.family: sans.name
                    }
                }
            }
        }
    }

    ColumnLayout {
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: root.border.width
            Layout.leftMargin: root.border.width
            Layout.rightMargin: root.border.width

            height: 25
            radius: 6
            color: {
                if (ma.containsPress) {
                    return Qt.darker(root.color, 1.2)
                } else if (ma.containsMouse) {
                    return Qt.darker(root.color, 1.1)
                } else {
                    return root.color
                } }

            // Inner rectangle to cover up the corner radius on the bottom edge
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                color: parent.color
                height: parent.radius
            }

            Text {
                id: titleLabel

                anchors.centerIn: parent
                text: root.title
            }

            MouseArea {
                id: ma

                property bool dragging: false
                property point startDragPos

                anchors.fill: parent
                hoverEnabled: true

                drag.onActiveChanged: {
                    dragging = !dragging
                    if (dragging) {
                        startDragPos = Qt.point(root.x, root.y)
                    } else {
                        root.xDrag += root.x - startDragPos.x
                        root.yDrag += root.y - startDragPos.y
                    }
                }

                drag.target: root
                drag.axis: Drag.XAndYAxis
                drag.threshold: 0
            }
        }

        RowLayout {
            spacing: 10
            property real margin: 15

            Loader {
                id: inputSockets

                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.topMargin: parent.margin
                Layout.bottomMargin: parent.margin
                Layout.leftMargin: -5

                sourceComponent: socketColumn
                onLoaded: {
                    item.sockets = inputs
                    item.floatRight = true
                }
            }

            Loader {
                Layout.topMargin: parent.margin
                Layout.bottomMargin: parent.margin
                Layout.minimumWidth: titleLabel.width

                sourceComponent: root.ui
            }

            Loader {
                id: outputSockets

                Layout.alignment: Qt.AlignBottom | Qt.AlignRight
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
}
