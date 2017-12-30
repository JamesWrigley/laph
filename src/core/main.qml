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
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import laph 0.1

import "."
import "../nodes"
import "../components"
import "main.js" as Main

ApplicationWindow {
    title: "Laph"
    visible: true
    width: Screen.width / 1.5
    height: Screen.height / 1.5
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2

    NodeMonitor {
        id: nm
        dir: "src/nodes"
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Shift+A"
        onActivated: addMenu.popup()
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "X"
        onActivated: canvas.deleteSelectedNode()
    }

    Menu {
        id: addMenu

        title: "Nodes"

        style: MenuStyle {
            frame: Rectangle {
                color: "#202020"
            }
        }

        MenuItem { text: "Nodes"; enabled: false }
        MenuSeparator { }
        Instantiator {
            model: nm.nodes
            onObjectAdded: addMenu.insertItem(index, object)
            onObjectRemoved: addMenu.removeItem(object)

            MenuItem {
                text: modelData.slice(0, -4) // Remove the file extension
                onTriggered: canvas.addNode(nm.dir + "/" + modelData,
                                            mouseArea.mouseX, mouseArea.mouseY)
            }
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        focus: FocusSingleton.canvasFocus

        property var nodes: new Array()
        property real xOffset: 0
        property real yOffset: 0
        property real scaling: 1
        property int nodeCount: 0
        property bool controlPressed: false

        function nodeHigherAt(point, srcNode) {
            for (var i = 0; i < nodes.length; ++i) {
                var node = nodes[i]
                var mappedPoint = node.mapFromItem(srcNode, point.x, point.y)
                if (node.contains(mappedPoint) && node.z > srcNode.z) {
                    return true
                }
            }

            return false
        }

        function deleteSelectedNode() {
            if (FocusSingleton.selectedNode != -1) {
                for (var i = 0; i < nodes.length; ++i) {
                    if (nodes[i].index == FocusSingleton.selectedNode) {
                        FocusSingleton.selectedNode = -1
                        nodes[i].destroy()
                        nodes.splice(i, 1)
                        canvas.requestPaint()
                    }
                }
            }
        }

        function addNode(nodeFile, x, y) {
            var nodeComponent = Qt.createComponent(nodeFile)
            if (nodeComponent.status == Component.Ready) {
                var node = nodeComponent.createObject(canvas, {"index": nodeCount,
                                                               "xDrag": x,
                                                               "yDrag": y,
                                                               "xOffset": Qt.binding(function() { return canvas.xOffset }),
                                                               "yOffset": Qt.binding(function() { return canvas.yOffset }),
                                                               "canvas":  Qt.binding(function() { return canvas })})
                if (node != null) {
                    nodeCount += 1
                    nodes.push(node)
                } else {
                    console.error("Node object", nodeFile, "could not be created")
                }
            } else {
                console.error("Component", nodeFile, "is not ready")
            }
        }

        Keys.onPressed: {
            if (event.modifiers & Qt.ControlModifier) {
                controlPressed = true
            }

            // Home key: center canvas
            if (event.key == Qt.Key_Home) {
                canvas.scaling = 1
                xOffset = 0
                yOffset = 0
                requestPaint()
            } else if (event.key == Qt.Key_Q &&
                       event.modifiers & Qt.ControlModifier) { // Ctrl + Q: Quit
                    Qt.quit()
            }
        }

        Keys.onReleased: {
            if (event.key & Qt.Key_Control) {
                controlPressed = false
            }
        }

        MouseArea {
            anchors.fill: parent

            acceptedButtons: Qt.AllButtons & ~Qt.MiddleButton

            onClicked: {
                parent.focus = true
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            hoverEnabled: true
            acceptedButtons: Qt.MiddleButton

            property real oldX
            property real oldY
            property real step: 0.01

            onPositionChanged: {
                if (pressed) {
                    parent.xOffset += mouse.x - oldX
                    parent.yOffset += mouse.y - oldY
                    parent.requestPaint()
                }

                oldX = mouse.x
                oldY = mouse.y
            }

            onWheel: {
                if (parent.controlPressed) {
                    var delta = wheel.angleDelta
                    var maxDelta = Math.abs(delta.x) > Math.abs(delta.y) ? delta.x : delta.y

                    if (Math.abs(maxDelta) > 0) {
                        if (maxDelta < 0 && parent.scaling > 0.4) {
                            parent.scaling -= step
                        } else if (maxDelta > 0 && parent.scaling < 3) {
                            parent.scaling += step
                        }
                    }
                } else {
                    parent.xOffset += wheel.pixelDelta.x
                    parent.yOffset += wheel.pixelDelta.y
                }

                parent.requestPaint()
            }
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset()
            ctx.strokeStyle = "#353535";
            ctx.fillStyle = "#494949"
            Main.drawGrid(ctx, width, height, scaling, xOffset, yOffset);
            ctx.stroke();
        }
    }
}
