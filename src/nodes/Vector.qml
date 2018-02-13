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
    outputs: [["k̂", NodeItem.Scalar]]
    // hooks: ({
    //     "k": [ parseFloat(ui.text) ]
    // })

    property var elements: ListModel { }

    ColumnLayout {
        width: childrenRect.width

        ScalarInput {
            id: si

            Layout.alignment: Qt.AlignTop
            Keys.onReturnPressed: {
                if (text != "") {
                    root.elements.append({"data": parseFloat(text)})
                    text = ""
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
            Layout.alignment: Qt.AlignBottom

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

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: root.elements.remove(index)
                }
            }
        }

        Text {
            text: "%1 rows".arg(root.elements.count)
        }
    }
}
