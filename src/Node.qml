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
    property var canvas
    property real xDrag
    property real yDrag
    property real xOffset
    property real yOffset

    FontLoader {
        id: sans
        source: "fonts/FiraSans-Regular.otf"
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

                        Connections {
                            target: root.canvas

                            onPaint: {
                                var ctx = root.canvas.context
                                ctx.strokeStyle = "cyan"
                                ctx.path = wire
                                ctx.stroke()
                            }
                        }

                        Path {
                            id: wire

                            PathCurve { id: curve }
                        }

                        MouseArea {
                            anchors.fill: parent

                            onPressed: {
                                var origin = parent.mapToItem(root.parent,
                                                              parent.width / 2,
                                                              parent.height / 2)
                                wire.startX = origin.x
                                wire.startY = origin.y
                            }

                            onPositionChanged: {
                                if (pressed) {
                                    var tip = parent.mapToItem(root.parent, mouse.x, mouse.y)
                                    curve.x = tip.x
                                    curve.y = tip.y
                                    root.canvas.requestPaint()
                                }
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
                drag.minimumX: 0
                drag.minimumY: 0
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
