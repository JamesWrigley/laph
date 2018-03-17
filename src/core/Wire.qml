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

import Laph 0.1

import QtQuick 2.7

import "../components"

WireItem {
    id: root

    valid: ((startType == null || endType == null) ||
            (startType == Socket.Generic || endType == Socket.Generic) ||
            (isScalar(startType) == isScalar(endType)))
    inputNode: endParent == null ? null : choose(endParent.onLeft, endParent.node, startParent.node, null)
    outputNode: endParent == null ? null : choose(endParent.onLeft, startParent.node, endParent.node, null)
    inputSocket: endParent == null ? "" : choose(inputNode == endParent.node, endParent.socketName, startParent.socketName, "")
    outputSocket: endParent == null ? "" : choose(endParent.onLeft, startParent.socketName, endParent.socketName, "")

    function choose(condition, option1, option2, unity) {
        var choice = condition ? option1 : option2
        return choice == undefined ? unity : choice
    }

    property int dragging
    property int endDragging: 1
    property int startDragging: 2

    property real endX
    property real endY
    property var canvas
    property var outputTip
    property bool startOnLeft
    property real endUpdateHook
    property real startUpdateHook

    property var endParent: end.item.parent
    property var startParent: start.item.parent

    property int endIndex: -1
    property int startIndex: -1
    property var endTip: end.item
    property var endType: end.item.socketType
    property var startType: start.item.socketType

    onValidChanged: {
        if (inputNode != null && outputNode != null) {
            evaluateInput()
        }
    }
    onEndTypeChanged: canvas.requestPaint()
    onStartTypeChanged: canvas.requestPaint()

    states: [
        State {
            name: "setOutputTip"
            when: endParent != null

            PropertyChanges {
                target: root
                outputTip: endParent.onLeft ? start.item : end.item
            }
        }
    ]

    Connections {
        target: canvas

        onPaint: {
            var ctx = canvas.getContext("2d")
            ctx.strokeStyle = valid ? "lightsteelblue" : "red"
            ctx.lineWidth = 2 * canvas.scaling
            ctx.path = wire
            ctx.stroke()
        }
    }

    function removeSelf() {
        graphEngine.removeWire(root)
    }

    function isScalar(type) {
        return type == Socket.Scalar || type == Socket.ScalarInput
    }

    function disconnect() {
        if (outputNode != null) {
            xcom.wireDisconnected(outputNode.index, outputSocket)
        }
    }

    function connect() {
        xcom.wireConnected(outputNode.index, outputSocket)
    }

    function evaluateInput() {
        if (valid) {
            graphEngine.evaluateFrom(inputNode, [inputSocket])
        }
    }

    function setParent(wireTip) {
        var target = wireTip.Drag.target
        wireTip.parent = target
        wireTip.x = 0
        wireTip.y = 0

        var hook = Qt.binding(function () {
            return target.node == null ? 0 : target.node.x + target.node.y + canvas.scaling
        })

        if (wireTip == start.item) {
            root.startUpdateHook = hook
        } else {
            root.endUpdateHook = hook
        }

        connect()
    }

    function handleRelease(wireTip) {
        if (wireTip.Drag.target != null && wireTip.Drag.drop() == Qt.MoveAction) {
            var justCreated = endParent == end
            if (!justCreated && wireTip == end.item) {
                disconnect()
            }

            root.setParent(wireTip)
            evaluateInput()
        } else {
            // We found that disconnecting wires from the output tip would for
            // some reason disable the DropArea on the inputTip socket, which we
            // fix by resetting the inputTip parent before the wire is deleted.
            var inputTip = outputTip == start.item ? end.item : start.item
            inputTip.parent = this

            canvas.requestPaint()
            removeSelf()

            if (outputNode != null) {
                disconnect()
            }
        }

        root.dragging = (~startDragging) & (~endDragging)
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
            property bool startOnLeft: start.item.parent.onLeft == undefined ? false : start.item.parent.onLeft
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
            property var socketName: parent == null ? undefined : parent.socketName

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
