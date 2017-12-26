import QtQuick 2.7

import "../components"

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
    property var endTip: end.item

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

    function handleRelease(wireTip) {
        if (wireTip.Drag.target != null && wireTip.Drag.drop() == Qt.MoveAction) {
            root.dragging = false
            root.setParent(wireTip)
        } else {
            root.destroy()
            canvas.requestPaint()
        }
    }

    Connections {
        target: canvas

        onPaint: {
            var ctx = canvas.getContext("2d")
            ctx.strokeStyle = "lightsteelblue"
            ctx.lineWidth = 2
            ctx.path = wire
            ctx.stroke()
        }
    }

    function computeCoord(wireTip, hook, x) {
        var oldCoord = (x ? wireTip.x : wireTip.y) + wireTip.width / 2
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

        PathCubic {
            id: cubic
            x: computeCoord(end.item, endUpdateHook, true)
            y: computeCoord(end.item, endUpdateHook, false)
            relativeControl1X: startOnLeft ? Math.max(relative1X, sturdiness) : Math.min(relative1X, -sturdiness)
            relativeControl1Y: 0
            relativeControl2X: startOnLeft ? Math.min(relative2X, relative2X - sturdiness) : Math.max(relative2X, relative2X + sturdiness)
            relativeControl2Y: y - wire.startY

            property int curvature: 2
            property real relative2X: x - wire.startX
            property real relative1X: (x - wire.startX) / curvature
            property bool startOnLeft: start.item.parent.onLeft
            property real sturdiness: Math.abs(wire.startX - x) / curvature

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
            opacity: 0

            Drag.active: true
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            property int twinIndex
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
                    root.handleRelease(parent)
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
            item.twinIndex = Qt.binding(function () { return startIndex })
        }
    }
}
