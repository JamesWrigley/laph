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
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

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
        dir: "./src/qml/nodes"
    }

    Connections {
        target: xcom

        onRepaintCanvas: canvas.requestPaint()
        onCreateNode: canvas.addNode(nodeFile, index, x, y)
        onDeleteNode: canvas.deleteNode(index)
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Shift+A"
        onActivated: addMenu.popup()
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "X"
        onActivated: {
            if (FocusSingleton.selectedNode != -1) {
                xcom.eventId += 1
                xcom.requestDeleteNode(FocusSingleton.selectedNode)
            }
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Space"
        onActivated: {
            searchBox.x = mouseArea.mouseX
            searchBox.y = mouseArea.mouseY
            searchBox.visible = true
        }
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Z"
        onActivated: xcom.requestUndo()
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequence: "Ctrl+Shift+Z"
        onActivated: xcom.requestRedo()
    }

    SearchBox {
        id: searchBox

        z: FocusSingleton.maxZ + 1
        nodes: nm.nodes

        onSelected: {
            xcom.eventId += 1
            xcom.requestCreateNode(node, canvas.nodeCount, mouseArea.mouseX, mouseArea.mouseY)
        }

        Component.onCompleted: {
            // If we don't do this after the component is loaded, QML throws an error
            visible = false
        }
    }

    // Menu {
    //     id: addMenu
    // 
    //     title: "Nodes"
    // 
    //     style: MenuStyle {
    //         frame: Rectangle {
    //             color: "#202020"
    //         }
    //     }
    // 
    //     MenuItem { text: "Nodes"; enabled: false }
    //     MenuSeparator { }
    //     Instantiator {
    //         model: nm.nodes
    //         onObjectAdded: addMenu.insertItem(index, object)
    //         onObjectRemoved: addMenu.removeItem(object)
    // 
    //         MenuItem {
    //             text: modelData.slice(0, -4) // Remove the file extension
    //             onTriggered: canvas.addNode(modelData, mouseArea.mouseX, mouseArea.mouseY)
    //         }
    //     }
    // }

    Canvas {
        id: canvas
        anchors.fill: parent

        focus: FocusSingleton.canvasFocus

        property var nodes: new Array()
        property int nodeCount: 0
        property bool shiftPressed: false
        property bool controlPressed: false

        property real scaling: 1
        property real xOffset: 0
        property real yOffset: 0

        Item {
            id: dummyNode
            enabled: false
            z: 0
        }

        Item {
            id: nodesContainer

            z: 1
            transform: Scale {
                xScale: canvas.scaling
                yScale: canvas.scaling

                origin.x: canvas.width / 2
                origin.y: canvas.height / 2
            }
        }

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

        function getSocket(point) {
            for (var i = 0; i < nodes.length; ++i) {
                var node = nodes[i];
                var socket = node.getSocketAt(point)

                if (socket != null) {
                    return socket
                }
            }

            return null
        }

        function deleteNode(nodeIndex) {
            if (FocusSingleton.selectedNode === nodeIndex) {
                FocusSingleton.selectedNode = -1;
                FocusSingleton.canvasFocus = true
            }

            for (var i = 0; i < nodes.length; ++i) {
                if (nodes[i].index == nodeIndex) {
                    graphEngine.removeNode(nodeIndex)
                    nodes.splice(i, 1)
                    xcom.repaintCanvas()
                    break
                }
            }
        }

        function addNode(nodeFile, index, x, y) {
            var path = nm.dir + "/" + nodeFile
            var node = graphEngine.beginCreateNode(path)

            node.parent = nodesContainer
            node.index = index
            node.xDrag = x - xOffset
            node.yDrag = y - yOffset
            node.rootZ = FocusSingleton.maxZ
            node.scaling = scaling
            node.xOffset = Qt.binding(function() { return canvas.xOffset })
            node.yOffset = Qt.binding(function() { return canvas.yOffset })
            node.canvas =  Qt.binding(function() { return canvas })

            var jlPath = "./jl/" + nodeFile.replace("qml", "jl")
            graphEngine.endCreateNode(jlPath, node)
            nodeCount += 1
            nodes.push(node)
        }

        Keys.onPressed: {
            if (event.modifiers & Qt.ControlModifier) {
                controlPressed = true
            }
            if (event.modifiers & Qt.ShiftModifier) {
                shiftPressed = true
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
            if (event.key & Qt.Key_Shift) {
                shiftPressed = false
            }
        }

        MouseArea {
            anchors.fill: parent

            acceptedButtons: Qt.AllButtons & ~Qt.MiddleButton

            onClicked: {
                if (!parent.nodeHigherAt(Qt.point(mouse.x, mouse.y),
                                         dummyNode)) {
                    FocusSingleton.canvasFocus = true
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            hoverEnabled: true
            acceptedButtons: Qt.MiddleButton

            property real oldX
            property real oldY
            property real step: 0.05

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
                if (parent.controlPressed && parent.shiftPressed) {
                    return
                }

                var delta = wheel.angleDelta
                var maxDelta = Math.abs(delta.x) > Math.abs(delta.y) ? delta.x : delta.y

                if (parent.controlPressed) {
                    parent.xOffset += maxDelta
                }
                if (parent.shiftPressed) {
                    parent.yOffset += maxDelta
                }

                if (!parent.shiftPressed && !parent.controlPressed) {
                    if (Math.abs(maxDelta) > 0) {
                        if (maxDelta < 0 && parent.scaling > 0.4) {
                            parent.scaling -= step
                        } else if (maxDelta > 0 && parent.scaling < 3) {
                            parent.scaling += step
                        }
                    }
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
