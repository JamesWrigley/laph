import QtQuick 2.7

Item {
    id: root

    property real endX
    property real endY
    property var canvas
    property bool initialRelease

    Connections {
        target: canvas

        onPaint: {
            var ctx = canvas.context
            ctx.strokeStyle = "cyan"
            ctx.path = wire
            ctx.stroke()
        }
    }

    Path {
        id: wire

        startX: start.mapToItem(canvas, start.item.x + start.item.width / 2, 0).x
        startY: start.mapToItem(canvas, 0, start.item.y + start.item.height / 2).y

        onStartXChanged: canvas.requestPaint()
        onStartYChanged: canvas.requestPaint()

        PathCurve {
            id: curve

            x: end.mapToItem(canvas, end.item.x + end.item.width / 2, 0).x
            y: end.mapToItem(canvas, 0, end.item.y + end.item.height / 2).y

            onXChanged: canvas.requestPaint()
            onYChanged: canvas.requestPaint()
        }
    }

    Component {
        id: tip

        Rectangle {
            width: 14
            height: 14
            color: "green"
            opacity: 0.5

            Drag.active: ma.Drag.active
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            Connections {
                target: root

                onInitialReleaseChanged: {
                    if (!initialRelease && parent.Drag.target == null) {
                        root.destroy()
                        canvas.requestPaint()
                    } else {
                        initialRelease = true
                    }
                }
            }

            MouseArea {
                id: ma
                anchors.fill: parent

                drag.target: parent
                drag.axis: Drag.XAndYAxis
                drag.threshold: 0
            }
        }
    }

    Loader {
        id: start
        sourceComponent: tip

        onLoaded: {
            item.parent = Qt.binding(function () { return parent.parent })
        }
    }

    Loader {
        id: end
        sourceComponent: tip

        onLoaded: {
            item.x = Qt.binding(function () { return endX - item.width / 2 })
            item.y = Qt.binding(function () { return endY - item.height / 2 })
        }
    }
}
