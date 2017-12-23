import QtQuick 2.7
import "."

Item {
    id: root

    property int endIndex: -1
    property int startIndex: -1

    property real endX
    property real endY
    property var canvas
    property bool dragging
    property real endUpdateHook
    property real startUpdateHook

    function setEndParent(endParent) {
        end.item.parent = endParent
        end.item.x = 0
        end.item.y = 0
    }

    function setParent(wireTip) {
        var target = wireTip.Drag.target
        wireTip.parent = target
        wireTip.x = 0
        wireTip.y = 0

        var hook = Qt.binding(function () {
            return target.node.x + target.node.y
        })

        if (wireTip == start.item) {
            root.startUpdateHook = hook
        } else {
            root.endUpdateHook = hook
        }
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

            property int twinIndex

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
                    if (parent.Drag.target != null && parent.Drag.drop() == Qt.MoveAction) {
                        root.dragging = false
                        root.setParent(parent)
                    } else {
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
            item.twinIndex = Qt.binding(function () { return endIndex })
        }
    }

    Loader {
        id: end
        sourceComponent: tip

        onLoaded: {
            item.x = Qt.binding(function () { return endX })
            item.y = Qt.binding(function () { return endY })
            item.twinIndex = Qt.binding(function () { return startIndex })
        }
    }
}
