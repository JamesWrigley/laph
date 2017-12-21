import QtQuick 2.7

Item {
    id: root

    property real endX
    property real endY
    property var canvas
    property bool dragging
    property real endUpdateHook: 0
    property real startUpdateHook: 0

    function setEndParent(endParent) {
        end.item.parent = endParent
    }

    Connections {
        target: canvas

        onPaint: {
            var ctx = canvas.context
            ctx.strokeStyle = "cyan"
            ctx.path = wire
            ctx.stroke()
        }
    }

    function computeCoord(wireTip, hook, x) {
        var oldCoord = x ? wireTip.x : wireTip.y
            + (x ? wireTip.width : wireTip.height) / 2
        var newCoord = (dragging ? wireTip.parent : wireTip)
            .mapToItem(root.canvas, x ? oldCoord : hook, x ? hook : oldCoord)

        if (root.dragging && wireTip != start.item && x) {
            var coord = end.item.mapToItem(root.canvas, endX, 0)
            console.info(newCoord.x, endX, coord.x)
        }

        return x ? newCoord.x : newCoord.y
    }

    Path {
        id: wire

        startX: computeCoord(start.item, startUpdateHook, true)
        startY: computeCoord(start.item, startUpdateHook, false)

        onStartXChanged: canvas.requestPaint()            
        onStartYChanged: canvas.requestPaint()

        PathCurve {
            id: curve

            x: computeCoord(end.item, endUpdateHook, true)
            y: computeCoord(end.item, endUpdateHook, false)

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

            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.active: ma.drag.active

            property var mouseArea: ma

            MouseArea {
                id: ma
                anchors.fill: parent

                drag.threshold: 0
                drag.target: parent
                drag.axis: Drag.XAndYAxis

                onPressed: {
                    root.dragging = true
                }

                onReleased: {
                    root.dragging = false

                    if (parent.Drag.target == null) {
                        root.destroy()
                        canvas.requestPaint()
                    }
                }
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
            item.x = Qt.binding(function () { return endX })
            item.y = Qt.binding(function () { return endY })
        }
    }
}
