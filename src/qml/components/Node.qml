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
import QtQuick.Controls 2.4

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
    property color mainColor: Qt.rgba(0.33, 0.33, 0.33, 0.8)

    signal inputChanged()

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

    function createWire(dropArea, mouseArea, isInput) {
        var wire = glode.beginCreateWire()

        // Wire configuration
        wire.initialSocket = dropArea
        wire.startIndex = glode.index
        wire.startOnLeft = isInput
        wire.canvas = Qt.binding(function () { return canvas })
        wire.startUpdateHook = Qt.binding(function () {
            return glode.x + glode.y + canvas.scaling
        })
        wire.endUpdateHook = Qt.binding(function () {
            return mouseArea.mouseX + mouseArea.mouseY
        })

        glode.endCreateWire()
        mouseArea.wire = wire

        return wire
    }

    function getObjectName(type, name, socketIsInput) {
        return type + "_" + name + "_" + (socketIsInput ? "I" : "O")
    }

    FocusScope {
        id: scope

        Rectangle {
            id: root

            z: rootZ
            radius: 10
            focus: true
            color: mainMa.drag.active ? Qt.lighter(glode.mainColor, 1.25) : glode.mainColor
            border.width: 4
            border.color: selected ? Qt.lighter("gray", 1.5) : "gray"
            width: childrenRect.width
            height: childrenRect.height

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

            ToolTip {
                id: tooltip
                delay: 200
                width: 250
                visible: mainMa.containsMouse && tipListView.count > 0

                contentItem: Column {
                    padding: 0
                    spacing: 0

                    Repeater {
                        id: tipListView
                        model: glode.messages

                        Label {
                            id: msgLabel
                            text: msg
                            color: "gray"
                            padding: 10

                            background: Rectangle {
                                opacity: 0.5
                                color: "#202020"

                                Rectangle {
                                    opacity: 1
                                    color: Qt.tint(Qt.lighter(parent.color, 2), "red")
                                    height: 3
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                }
                            }
                        }
                    }
                }

                background: Rectangle { opacity: 0 }
            }

            MouseArea {
                id: mainMa

                anchors.fill: root
                hoverEnabled: true

                drag.target: glode
                drag.threshold: 0
                drag.axis: Drag.XAndYAxis
                drag.onActiveChanged: {
                    if (drag.active) {
                        oldX = glode.x
                        oldY = glode.y

                        startDragPos = Qt.point(glode.x, glode.y)
                    } else {
                        xDrag += glode.x - startDragPos.x
                        yDrag += glode.y - startDragPos.y

                        xcom.eventId += 1
                        xcom.nodeMoved(glode.index, oldX, oldY, glode.x, glode.y)
                    }
                }

                onPressed: {
                    root.attemptFocus(mouse.x, mouse.y)
                }

                onWheel: wheel.accepted = false
                onClicked: mouse.accepted = false
                onReleased: mouse.accepted = false
                onDoubleClicked: mouse.accepted = false
                onPositionChanged: mouse.accepted = false

                property int oldX
                property int oldY
                property point startDragPos
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
                                border.width: 1
                                border.color: Qt.darker(color, 2)
                                color: {
                                    if (isGeneric) {
                                        return da.containsDrag ? Qt.lighter("teal", 1.5) : "teal"
                                    } else if (isScalar) {
                                        return da.containsDrag ? Qt.lighter("purple", 1.5) : "purple"
                                    } else {
                                        return da.containsDrag ? Qt.lighter("green", 1.5) : "green"
                                    }
                                }

                                property alias isInput: da.isInput
                                property bool isScalar: type & Socket.Scalar
                                property bool isGeneric: type & Socket.Generic

                                DropArea {
                                    id: da
                                    objectName: getObjectName("da", name, !isInput)
                                    anchors.fill: parent

                                    property var node: glode
                                    property bool isInput: !floatRight
                                    property int wires: children.length
                                    property var socketType: type
                                    property string socketName: name
                                    property int socketChangeHook: socketRepeater.socketChangeHook

                                    onDropped: {
                                        if (drop.source.twinIndex !== glode.index &&
                                            drop.source.isOutput === !isInput) {

                                            drop.accept(Qt.MoveAction)
                                        }
                                    }
                                }

                                MouseArea {
                                    id: ma
                                    objectName: getObjectName("ma", name, !parent.isInput)
                                    anchors.fill: parent

                                    drag.target: wire === null ? undefined : wire.endTip
                                    drag.threshold: 0
                                    drag.axis: Drag.XAndYAxis

                                    property var wire: null

                                    onPressed: {
                                        if (da.wires === 0) {
                                            xcom.eventId += 1
                                            xcom.requestCreateWire(glode.index, name, !parent.isInput)
                                            wire.endTip.Drag.active = true
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

                Text {
                    id: titleLabel
                    Layout.topMargin: 10
                    Layout.bottomMargin: -10
                    Layout.alignment: Qt.AlignHCenter

                    text: title
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
