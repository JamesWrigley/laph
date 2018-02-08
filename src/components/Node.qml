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

    x: xDrag + xOffset
    y: yDrag + yOffset
    z: root.z

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
    property real xOffset
    property real yOffset
    default property Component uiComponent
    property bool selected: FocusSingleton.selectedNode == index

    signal inputChanged()

    Connections {
        target: graphEngine

        onInputChanged: {
            if (nodeIndex == glode.index) {
                inputChanged()
            }
        }
    }

    function input(socketName) {
        return graphEngine.inputAsString(glode, socketName)
    }

    Rectangle {
        id: root

        z: rootZ
        radius: 10
        opacity: 0.8
        color: "#555555"
        border.width: 4
        border.color: selected ? Qt.lighter("gray", 1.5) : "gray"
        width: childrenRect.width
        height: childrenRect.height

        function input(socketName) {
            return glode.input(socketName)
        }

        FontLoader {
            id: sans
            source: "../fonts/FiraSans-Regular.otf"
        }

        MouseArea {
            // Ugly hack to make sure that this overlays all other MouseArea's
            z: 1 + parent.z
            anchors.fill: mainLayout

            onClicked: mouse.accepted = false
            onWheel: wheel.accepted = false
            onPressed: {
                if (!canvas.nodeHigherAt(Qt.point(mouse.x, mouse.y), parent)) {
                    if (FocusSingleton.selectedNode != glode.index) {
                        FocusSingleton.selectedNode = glode.index
                        parent.z = FocusSingleton.maxZ
                    }

                    mouse.accepted = false
                } else {
                    mouse.accepted = true
                }
            }
            onReleased: mouse.accepted = false
            onDoubleClicked: mouse.accepted = false
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
                            border.width: 1
                            border.color: Qt.darker(color, 2)
                            color: modelData[1] == NodeItem.Scalar ? "purple" : "green"

                            property alias onLeft: da.onLeft

                            DropArea {
                                id: da
                                anchors.fill: parent

                                property var node: glode
                                property bool onLeft: !floatRight
                                property int wires: children.length
                                property var socketType: modelData[1]
                                property string socketName: modelData[0]

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

                                width: parent.width
                                height: parent.height

                                drag.target: wire == null ? undefined : wire.endTip
                                drag.threshold: 0
                                drag.axis: Drag.XAndYAxis
                                enabled: wire == null && da.wires == 0

                                property var wire: null

                                onPressed: {
                                    var component = Qt.createComponent("../core/Wire.qml")
                                    if (component.status == Component.Ready) {
                                        wire = component.createObject(da, {"startIndex": glode.index,
                                                                           "startOnLeft": parent.onLeft,
                                                                           "canvas": Qt.binding(function () { return canvas }),
                                                                           "startUpdateHook": Qt.binding(function () {
                                                                               return glode.x + glode.y
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
                            text: modelData[0]
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

                    anchors.fill: parent

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
                        item.sockets = glode.inputs
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
                        item.sockets = glode.outputs
                        item.floatRight = false
                    }
                }
            }
        }
    }
}
