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

import QtQuick 2.11

import "../components"
import "Wire.js" as Wire

WireItem {
    id: root

    valid: ((startType === null || endType === null) ||
            ((startType & Socket.Generic) || (endType & Socket.Generic)) ||
            ((startType & Socket.Scalar) === (endType & Socket.Scalar)))
    inputNode: null
    outputNode: null
    inputSocket: ""
    outputSocket: ""
    endParent: end.item.parent
    initialSocket: null

    property real endX
    property real endY
    property var canvas
    property bool startOnLeft
    property real endUpdateHook
    property real startUpdateHook

    property var startParent: start.item.parent

    property int endIndex: -1
    property int startIndex: -1
    property var endTip: end.item
    property var startTip: start.item
    property var endType: end.item.socketType
    property var startType: start.item.socketType

    Connections {
        target: xcom

        onConnectWireTip: {
            if (wireTip.wireIndex === root.index) {
                Wire.handleConnect(wireTip, target, type, true)
                wireTip.Drag.active = false
            }
        }
        onReconnectWireTip: {
            if (wireTip.wireIndex === root.index) {
                Wire.handleConnect(wireTip, target, type, isReplay)
            }
        }
        onDeleteWire: {
            if (index == root.index) {
                Wire.deleteWire()
            }
        }
    }

    onValidChanged: {
        if (inputNode !== null && outputNode !== null) {
            Wire.evaluateInput()
        }
    }
    onEndTypeChanged: xcom.repaintCanvas()
    onStartTypeChanged: xcom.repaintCanvas()

    function handleRelease(wireTip) {
        var target = wireTip.Drag.target
        if (target !== null && wireTip.Drag.drop() == Qt.MoveAction) {
            var connectionType = null
            if (endParent === initialSocket) {
                connectionType = XCom.ConnectionType.New
            } else if (wireTip.parent === target) {
                connectionType = XCom.ConnectionType.None
            } else {
                // Only reconnects are handled entirely through C++, so all we
                // do here is emit XCom.requestReconnectWireTip().
                var tipType = wireTip.socketName == root.inputSocket ? XCom.TipType.Input : XCom.TipType.Output
                xcom.requestReconnectWireTip(root.index, tipType, target.node.index, target.socketName)
                return
            }

            Wire.handleConnect(wireTip, target, connectionType, false)
        } else {
            Wire.handleDisconnect()
        }
    }

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

    Path {
        id: wire

        startX: Wire.computeCoord(start.item, startUpdateHook, true)
        startY: Wire.computeCoord(start.item, startUpdateHook, false)

        onStartXChanged: xcom.repaintCanvas()
        onStartYChanged: xcom.repaintCanvas()

        PathCubic {
            id: cubic

            x: Wire.computeCoord(end.item, endUpdateHook, true)
            y: Wire.computeCoord(end.item, endUpdateHook, false)
            relativeControl1X: startOnLeft ? Math.max(relative1X, sturdiness) : Math.min(relative1X, -sturdiness)
            relativeControl1Y: 0
            relativeControl2X: startOnLeft ? Math.min(relative2X, relative2X - sturdiness) : Math.max(relative2X, relative2X + sturdiness)
            relativeControl2Y: y - wire.startY

            property int curvature: 2
            property real relative1X: (x - wire.startX) / curvature
            property real relative2X: x - wire.startX
            property bool startOnLeft: startParent === null ? false : startParent.isInput
            property real sturdiness: Math.abs(wire.startX - x) / curvature

            onXChanged: xcom.repaintCanvas()
            onYChanged: xcom.repaintCanvas()
        }
    }

    Component {
        id: tip

        Rectangle {
            width: 14
            height: 14

            // When debugging, it's useful to dial the opacity up and visualize
            // the tip as a rectangle.
            color: "green"
            opacity: 0

            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            property int twinIndex
            property bool isOutput
            property int wireIndex: root.index
            property int index: root.startIndex
            property var socketType: parent === null ? undefined : parent.socketType
            property var socketName: parent === null ? undefined : parent.socketName

            MouseArea {
                id: ma
                anchors.fill: parent

                drag.threshold: 0
                drag.target: parent
                drag.axis: Drag.XAndYAxis

                onPressed: {
                    parent.Drag.active = true
                }

                onReleased: {
                    root.handleRelease(parent)
                    parent.Drag.active = false
                }
            }
        }
    }

    Loader {
        id: start
        sourceComponent: tip

        onLoaded: {
            item.parent = initialSocket
            item.isOutput = Qt.binding(function () { return !root.startOnLeft })
            item.twinIndex = Qt.binding(function () { return endIndex })
        }
    }

    Loader {
        id: end
        sourceComponent: tip

        onLoaded: {
            item.Drag.active = true

            item.parent = initialSocket
            item.isOutput = Qt.binding(function () { return root.startOnLeft })
            item.twinIndex = Qt.binding(function () { return startIndex })
        }
    }
}
