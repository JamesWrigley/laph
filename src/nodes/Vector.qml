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
import QtQml.Models 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import "../components"

Node {
    id: root

    title: "Vector"
    inputs: []
    outputs: [[key, NodeItem.VectorInput]]
    hooks: QtObject {
        property var k̂: new Array()
    }

    property string key: "k̂"
    property var elements: ListModel { }

    ColumnLayout {
        width: childrenRect.width

        ScalarInput {
            id: si

            Keys.onReturnPressed: {
                if (text != "") {
                    var num = parseFloat(text)

                    root.elements.append({"data": num})
                    hooks[key].push(num)

                    text = ""
                    nodeChanged(root, [key])
                }
            }
        }

        ListView {
            id: lv

            clip: true
            spacing: 1
            currentIndex: model.count - 1

            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 125)

            ScrollBar.vertical: ScrollBar {
                parent: lv.parent

                active: true
                wheelEnabled: true

                anchors.top: lv.top
                anchors.right: lv.right
                anchors.bottom: lv.bottom
            }

            model: root.elements
            delegate: ScalarInput {
                radius: 1
                text: modelData

                // For some reason the MouseArea doesn't have root in it's
                // scope, so we use this binding instead.
                property var subRoot: root

                Keys.onReturnPressed: {
                    if (text != "") {
                        hooks[key][index] = parseFloat(text)
                        nodeChanged(root, [key])
                    }

                    event.accepted = false
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        hooks[key].splice(index, 1)
                        subRoot.elements.remove(index)
                        subRoot.nodeChanged(subRoot, [subRoot.key])
                    }
                }
            }
        }

        Text {
            text: "%1 rows".arg(root.elements.count)
        }
    }
}
