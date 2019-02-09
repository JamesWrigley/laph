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
import QtQuick.Controls 2.2

import "../components"

Node {
    title: "Result"
    inputs: ({y: { type: Socket.Generic | Socket.Input }})
    outputs: ({ })

    property var inputValueType: Socket.Scalar | Socket.Input

    function setInputType(dataType) {
        inputValueType = dataType | Socket.Input
    }

    onInputChanged: {
        setInputType(graphEngine.getInputValueType(this, "y"))
        var inputValue = input("y")

        if (inputValueType & Socket.Scalar) {
            ui.item.text = inputValue
        } else {
            var vec = inputValue

            ui.item.model.clear()
            for (var i = 0; i < vec.length; ++i) {
                ui.item.model.append({"data": vec[i]})
            }
        }
    }

    Component {
        id: scalarOutput

        Output { }
    }

    Component {
        id: vectorOutput
    
        ColumnLayout {
            width: 125

            property var model: ListModel { }

            ListView {
                id: lv
                
                clip: true
                spacing: 1
                model: parent.model

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
                
                delegate: Output {
                    radius: 1
                    text: modelData
                }
            }

            Text {
                text: "%1 rows".arg(model.count)
            }
        }
    }

    Loader {
        sourceComponent: inputValueType & Socket.Scalar ? scalarOutput : vectorOutput
    }
}
