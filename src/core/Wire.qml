/*************************** Copyrights and License *******************************
 *                                                                                *
 * This file is part of Laph. http://github.com/JamesWrigley/laph/                *
 *                                                                                *
 * Laph is free software: you can redistribute it and/or modify it under          *
 * the terms of the GNU General Public License as published by the Free Software  *
 * Foundation, either version 3 of the License, or (at your option) any later     *
 * version.                                                                       *
 *                                                                                *
 * Laph is distributed in the hope that it will be useful, but WITHOUT ANY        *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS      *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. *
 *                                                                                *
 * You should have received a copy of the GNU General Public License along with   *
 * Laph. If not, see <http://www.gnu.org/licenses/>.                              *
 *                                                                                *
 *********************************************************************************/

import QtQuick 2.7

import "../components"

Item {
    id: root

    property int dragging
    property int endDragging: 1
    property int startDragging: 2

    property real endX
    property real endY
    property var canvas
    property bool startOnLeft
    property real endUpdateHook
    property real startUpdateHook

    property int endIndex: -1
    property int startIndex: -1
    property var endTip: end.item
    property var endType: end.item.socketType
    property var startType: start.item.socketType

    function setParent(wireTip) {
        var target = wireTip.Drag.target
        wireTip.parent = target
        wireTip.x = 0
        wireTip.y = 0

        var hook = Qt.binding(function () {
            return target.node == null ? 0 : target.node.x + target.node.y
        })

        if (wireTip == start.item) {
            root.startUpdateHook = hook
        } else {
            root.endUpdateHook = hook
        }
    }

    function handleRelease(wireTip) {
        if (wireTip.Drag.target != null && wireTip.Drag.drop() == Qt.MoveAction) {
            root.setParent(wireTip)
        } else {
            root.destroy()
            canvas.requestPaint()
        }
        root.dragging = (~startDragging) & (~endDragging)
    }

    Connections {
        target: canvas

        onPaint: {
            var ctx = canvas.getContext("2d")
            ctx.strokeStyle = ((startType == null || endType == null) ||
                               (startType == endType)) ? "lightsteelblue" : "red"
            ctx.lineWidth = 2
            ctx.path = wire
            ctx.stroke()
        }
    }

    function computeCoord(wireTip, hook, x) {
        var oldCoord = (x ? wireTip.x : wireTip.y) + wireTip.width / 2
        var newCoord = (dragging > 0 ? wireTip.parent : wireTip)
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
            property real relative1X: (x - wire.startX) / curvature
            property real relative2X: x - wire.startX
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

            Drag.active: dragMask & root.dragging > 0
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            property int dragMask
            property int twinIndex
            property bool twinSide
            property var mouseArea: ma
            property var socketType: parent == null ? undefined : parent.socketType

            function destroyWire() {
                root.destroy()
            }

            MouseArea {
                id: ma
                anchors.fill: parent

                drag.threshold: 0
                drag.target: parent
                drag.axis: Drag.XAndYAxis

                onPressed: {
                    root.dragging = dragMask
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
            item.dragMask = startDragging
            item.parent = Qt.binding(function () { return parent.parent })
            item.twinSide = Qt.binding(function () { return !root.startOnLeft })
            item.twinIndex = Qt.binding(function () { return endIndex })
        }
    }

    Loader {
        id: end
        sourceComponent: tip

        onLoaded: {
            root.dragging = startDragging & (~endDragging)

            item.dragMask = endDragging
            item.twinSide = Qt.binding(function () { return root.startOnLeft })
            item.twinIndex = Qt.binding(function () { return startIndex })
        }
    }
}
