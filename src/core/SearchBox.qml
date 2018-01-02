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
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import "../components"

Rectangle {
    id: root

    opacity: 0.9
    width: mainLayout.width + margin
    height: mainLayout.height + margin

    color: "#202020"
    radius: 3

    property var nodes
    property int margin: 10

    signal selected(string node)

    function cleanup() {
        input.text = ""
        visible = false
    }

    ColumnLayout {
        id: mainLayout
        anchors.centerIn: parent
        height: 150

        Input {
            id: input

            Layout.fillWidth: true
            placeholderText: "Search..."

            // Need to do a bit of a rigmarole here because editingFinished() is
            // emitted  before  accepted() is  emitted,  so  it will  perform  a
            // premature cleanup  before the  accepted() handler can  access the
            // current model. Hence why we need to store the old text to restore
            // it (if necessary) in the accepted() handler.
            property string oldText

            onVisibleChanged: {
                if (visible) {
                    oldText = ""
                    forceActiveFocus()
                } else {
                    focus = false
                }
            }
            onEditingFinished: {
                if (!focus && visible) {
                    oldText = text
                    cleanup()
                }
            }
            onAccepted: {
                if (oldText != "") {
                    text = oldText
                }

                if (listView.currentIndex != -1) {
                    root.selected(listView.currentItem.node)
                }

                cleanup()
            }

            Keys.onDownPressed: {
                listView.currentIndex = Math.min(listView.count - 1,
                                                 listView.currentIndex + 1)
            }
            Keys.onUpPressed: {
                listView.currentIndex = Math.max(0,
                                                 listView.currentIndex - 1)
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            style: ScrollViewStyle {
                handle: Rectangle {
                    implicitWidth: 15
                    color: "#505050"
                }

                scrollBarBackground: Rectangle {
                    implicitWidth: 15
                    color: "transparent"
                }

                decrementControl: null
                incrementControl: null
            }

            ListView {
                id: listView
                width: input.width
                model: searchNodes()

                function searchNodes() {
                    var matches = []
                    for (var i = 0; i < nodes.length; ++i) {
                        if (nodes[i].toLowerCase().startsWith(input.text.toLowerCase())) {
                            matches.push(nodes[i])
                        }
                    }

                    return matches
                }

                delegate: Rectangle {
                    width: parent.width
                    height: text.height + margin
                    color: ListView.isCurrentItem ? Qt.darker(root.color, 1.5) : root.color

                    property string node: modelData

                    Text {
                        id: text

                        anchors.leftMargin: margin / 2
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        color: "gray"
                        text: node.slice(0, -4)
                    }

                    MouseArea {
                        id: ma

                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered: {
                            listView.currentIndex = index
                        }
                        onClicked: {
                            root.cleanup()
                            root.selected(node)
                        }
                    }
                }
            }
        }
    }
}
