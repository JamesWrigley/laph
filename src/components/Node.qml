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
    inputTypeSwaps: new Array(inputs.length)
    outputTypeSwaps: new Array(outputs.length)

    signal inputChanged()

    Component.onCompleted: {
        function fillTypeSwap(sockets, fillInputs) {
            for (var i = 0; i < sockets.length; ++i) {
                if (sockets[i].length > 2) {
                    (fillInputs ? inputTypeSwaps : outputTypeSwaps)[i] = false
                }
            }
        }

        fillTypeSwap(inputs, true)
        fillTypeSwap(outputs, false)
    }

    Connections {
        target: graphEngine

        onInputChanged: {
            if (nodeIndex == glode.index) {
                inputChanged()
            }
        }
    }

    function initializeSockets(socketMap) {
        var sockets = Object.keys(socketMap)

        for (var i = 0; i < sockets.length; ++i) {
            var socketField = socketMap[sockets[i]]
            if (!("generic" in socketField)) {
                socketMap[sockets[i]]["generic"] = false
            }
            if (!("repeating" in socketField)) {
                socketMap[sockets[i]]["repeating"] = false
            }
        }

        return socketMap
    }

    function input(socketName) {
        var type = graphEngine.getInputValueType(glode, socketName)
        if (type == NodeItem.Scalar) {
            return graphEngine.inputToString(glode, socketName)
        } else if (type == NodeItem.Vector) {
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
                if (event.key == Qt.Key_T) {
                    swapType()
                }
            }

            function attemptFocus(x, y) {
                if (!glode.canvas.nodeHigherAt(Qt.point(x, y), root)) {
                    if (FocusSingleton.selectedNode != index) {
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
                        model: parent.sockets

                        RowLayout {
                            LayoutMirroring.enabled: !floatRight

                            Rectangle {
                                id: socket

                                width: 14
                                height: width
                                radius: width / 2
                                border.width: generic ? 1.25 : 1
                                border.color: generic ? "black" : Qt.darker(color, 2)
                                color: type == NodeItem.Generic ? "teal" : isScalar ? "purple" : "green"

                                property alias onLeft: da.onLeft
                                property bool isScalar: (type == NodeItem.Scalar ||
                                                         type == NodeItem.ScalarInput)

                                Connections {
                                    target: root
                                    onSwapType: {
                                        if (generic && ma.containsMouse) {
                                            if (socket.onLeft) {
                                                glode.outputTypeSwaps[index] = !glode.outputTypeSwaps[index]
                                            } else {
                                                glode.inputTypeSwaps[index] = !glode.inputTypeSwaps[index]
                                            }

                                            socket.isScalar = !socket.isScalar
                                        }
                                    }
                                }

                                DropArea {
                                    id: da
                                    anchors.fill: parent

                                    property var node: glode
                                    property bool onLeft: !floatRight
                                    property int wires: children.length
                                    property var socketType: {
                                        if (type == NodeItem.Generic) {
                                            return type
                                        } else if (parent.isScalar) {
                                            return NodeItem.Scalar
                                        } else {
                                            return NodeItem.Vector
                                        }
                                    }
                                    property string socketName: name

                                    function disconnectWire(wire) {
                                        if (wire == ma.wire) {
                                            wire.destroy()
                                        } else {
                                            wire.destroyWire()
                                        }
                                    }

                                    Component.onDestruction: {
                                        for (var i = 0; i < wires; ++i) {
                                            disconnectWire(children[i])
                                        }
                                    }

                                    onDropped: {
                                        if (drop.source.twinIndex != glode.index &&
                                            drop.source.twinSide == !onLeft) {
                                            if (!onLeft && wires != 0) {
                                                // If this is an input, replace the old
                                                // wire with the new one.
                                                disconnectWire(children[0])
                                            }

                                            drop.accept(Qt.MoveAction)
                                        }
                                    }
                                }

                                MouseArea {
                                    id: ma

                                    anchors.fill: parent
                                    hoverEnabled: true

                                    drag.target: wire == null ? undefined : wire.endTip
                                    drag.threshold: 0
                                    drag.axis: Drag.XAndYAxis

                                    property var wire: null

                                    onPressed: {
                                        if (wire == null && da.wires == 0) {
                                            var component = Qt.createComponent("../core/Wire.qml")
                                            if (component.status == Component.Ready) {
                                                wire = component.createObject(da, {"startIndex": glode.index,
                                                                                   "startOnLeft": parent.onLeft,
                                                                                   "canvas": Qt.binding(function () { return canvas }),
                                                                                   "startUpdateHook": Qt.binding(function () {
                                                                                       return glode.x + glode.y + canvas.scaling
                                                                                   }),
                                                                                   "endUpdateHook": Qt.binding(function () {
                                                                                       return ma.mouseX + ma.mouseY
                                                                                   })})

                                                if (wire == null) {
                                                    console.error("Object 'Wire.qml' could not be created")
                                                }
                                            } else {
                                                console.error("Component 'Wire.qml' is not ready:", component.errorString())
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

                                onEditingFinished: focus = false
                                Keys.onEscapePressed: focus = false
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

                    function createModel(dict) {
                        var keys = Object.keys(dict)
                        var model = Qt.createQmlObject("import QtQuick 2.2; ListModel { }",
                                                       parent)
                        for (var i = 0; i < keys.length; ++i) {
                            var element = dict[keys[i]]
                            element["name"] = keys[i]
                            model.append(element)
                        }

                        return model
                    }

                    Loader {
                        id: inputSockets

                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                        Layout.topMargin: parent.margin
                        Layout.bottomMargin: parent.margin
                        Layout.leftMargin: -5

                        sourceComponent: socketColumn
                        onLoaded: {
                            glode.inputs = glode.initializeSockets(glode.inputs)
                            item.sockets = parent.createModel(glode.inputs)
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
                            glode.outputs = glode.initializeSockets(glode.outputs)
                            item.sockets = parent.createModel(glode.outputs)
                            item.floatRight = false
                        }
                    }
                }
            }
        }
    }
}
