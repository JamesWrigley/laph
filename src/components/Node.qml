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
import QtQuick.Layouts 1.3

import "."
import "../core"

NodeItem {
    id: glode

    z: root.z
    x: xDrag + xOffset
    y: yDrag + yOffset

    width: root.width
    height: root.height

    // Public properties
    property string title
    property var ui: uiLoader.item

    // Private properties (subclassing nodes should not touch these)
    property var canvas
    property real xDrag
    property real yDrag
    property real rootZ
    property real scaling
    property real xOffset
    property real yOffset
    default property Component uiComponent
    property bool selected: scope.activeFocus

    signal inputChanged()

    SocketModel {
        id: inputsModel
        objectName: "inputsModel"

        socketsTemplate: inputs
    }

    SocketModel {
        id: outputsModel
        objectName: "outputsModel"

        socketsTemplate: outputs
    }

    Connections {
        target: graphEngine

        onInputChanged: {
            if (nodeIndex === glode.index) {
                inputChanged()
            }
        }
    }

    function input(socketName) {
        var type = graphEngine.getInputValueType(glode, socketName)
        if (type & Socket.Scalar) {
            return graphEngine.inputToString(glode, socketName)
        } else if (type & Socket.Vector) {
            return graphEngine.inputToList(glode, socketName)
        }
    }

    FocusScope {
        id: scope

        Rectangle {
            id: root

            z: rootZ
            radius: 10
            focus: true
            color: Qt.rgba(0.33, 0.33, 0.33, 0.8)
            border.width: 4
            border.color: selected ? Qt.lighter("gray", 1.5) : "gray"
            width: childrenRect.width
            height: childrenRect.height

            signal swapType()

            Keys.onPressed: {
                if (event.key === Qt.Key_T) {
                    swapType()
                }
            }

            function attemptFocus(x, y) {
                if (!glode.canvas.nodeHigherAt(Qt.point(x, y), root)) {
                    if (FocusSingleton.selectedNode !== index) {
                        FocusSingleton.selectedNode = index
                        root.z = FocusSingleton.maxZ
                    }

                    FocusSingleton.canvasFocus = false
                    scope.focus = true
                    return false
                } else {
                    return true
                }
            }

            function input(socketName) {
                return glode.input(socketName)
            }

            FontLoader {
                id: sans
                source: "../fonts/FiraSans-Regular.otf"
            }

            MouseArea {
                id: mainMa

                // Ugly hack to make sure that this overlays all other MouseArea's
                z: 1 + parent.z
                anchors.fill: mainLayout

                onPressed: {
                    mouse.accepted = parent.attemptFocus(mouse.x, mouse.y)
                }

                onWheel: wheel.accepted = false
                onClicked: mouse.accepted = false
                onReleased: mouse.accepted = false
                onDoubleClicked: mouse.accepted = false
                onPositionChanged: mouse.accepted = false
            }

            Component {
                id: socketColumn

                Column {
                    id: column

                    spacing: 10

                    property var sockets
                    property bool floatRight

                    Repeater {
                        id: socketRepeater
                        model: parent.sockets

                        property int socketChangeHook: 0

                        RowLayout {
                            LayoutMirroring.enabled: !floatRight

                            onYChanged: socketRepeater.socketChangeHook += 1

                            Rectangle {
                                id: socket

                                width: 14
                                height: width
                                radius: width / 2
                                border.width: generic ? 1.25 : 1
                                border.color: generic ? "black" : Qt.darker(color, 2)
                                color: type & Socket.Generic ? "teal" : isScalar ? "purple" : "green"

                                property alias isInput: da.isInput
                                property bool isScalar: type & Socket.Scalar

                                Connections {
                                    target: root
                                    onSwapType: {
                                        if (generic && ma.containsMouse) {
                                            type = type & Socket.Scalar ? Socket.Vector : Socket.Scalar
                                        }
                                    }
                                }

                                DropArea {
                                    id: da
                                    anchors.fill: parent

                                    property var node: glode
                                    property bool isInput: !floatRight
                                    property int wires: children.length
                                    property var socketType: type
                                    property string socketName: name
                                    property int socketChangeHook: socketRepeater.socketChangeHook

                                    onDropped: {
                                        if (drop.source.twinIndex !== glode.index &&
                                            drop.source.twinSide === !isInput) {

                                            drop.accept(Qt.MoveAction)
                                        }
                                    }
                                }

                                MouseArea {
                                    id: ma

                                    anchors.fill: parent

                                    drag.target: wire === null ? undefined : wire.endTip
                                    drag.threshold: 0
                                    drag.axis: Drag.XAndYAxis

                                    property var wire: null

                                    onPressed: {
                                        if (wire === null && da.wires === 0) {
                                            wire = glode.beginCreateWire()

                                            // Wire configuration
                                            wire.initialSocket = da
                                            wire.startIndex = glode.index
                                            wire.startOnLeft = parent.isInput
                                            wire.canvas = Qt.binding(function () { return canvas })
                                            wire.startUpdateHook = Qt.binding(function () {
                                                return glode.x + glode.y + canvas.scaling
                                            })
                                            wire.endUpdateHook = Qt.binding(function () {
                                                return ma.mouseX + ma.mouseY
                                            })

                                            glode.endCreateWire()

                                            if (wire === null) {
                                                console.error("Object 'Wire.qml' could not be created")
                                            } else {
                                                graphEngine.addWire(wire)
                                            }
                                        } else {
                                            mouse.accepted = false
                                        }
                                    }

                                    onReleased: {
                                        wire.handleRelease(wire.endTip)
                                    }
                                }
                            }

                            TextInput {
                                width: contentWidth
                                height: contentHeight

                                color: "#202020"
                                text: name
                                font.family: sans.name
                                validator: RegExpValidator { regExp: /^([a-zA-Z]|_)+\S*$/ }

                                onEditingFinished: {
                                    if (text.length === 0) {
                                        text = name
                                    } else {
                                        name = text
                                    }
                                    focus = false
                                }
                                Keys.onEscapePressed: {
                                    text = name
                                    focus = false
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                id: mainLayout

                Rectangle {
                    id: titleBar

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
                        text: title
                    }

                    MouseArea {
                        id: ma

                        anchors.fill: titleBar
                        parent: mainMa
                        hoverEnabled: true

                        drag.target: glode
                        drag.threshold: 0
                        drag.axis: Drag.XAndYAxis
                        drag.onActiveChanged: {
                            if (drag.active) {
                                startDragPos = Qt.point(glode.x, glode.y)
                            } else {
                                xDrag += glode.x - startDragPos.x
                                yDrag += glode.y - startDragPos.y
                            }
                        }

                        property point startDragPos

                        onWheel: wheel.accepted = false
                        onClicked: mouse.accepted = false
                        onPressed: {
                            root.attemptFocus(mouse.x, mouse.y)
                        }
                        onReleased: mouse.accepted = false
                        onDoubleClicked: mouse.accepted = false
                        onPositionChanged: mouse.accepted = false
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
                            item.sockets = Qt.binding(function() { return inputsModel })
                            item.floatRight = true
                        }
                    }

                    Loader {
                        id: uiLoader
                        Layout.topMargin: parent.margin
                        Layout.bottomMargin: parent.margin
                        Layout.minimumWidth: titleLabel.width

                        sourceComponent: uiComponent
                    }

                    Loader {
                        id: outputSockets

                        Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                        Layout.topMargin: parent.margin
                        Layout.bottomMargin: parent.margin
                        Layout.rightMargin: -5

                        sourceComponent: socketColumn
                        onLoaded: {
                            item.sockets = Qt.binding(function() { return outputsModel })
                            item.floatRight = false
                        }
                    }
                }
            }
        }
    }
}
