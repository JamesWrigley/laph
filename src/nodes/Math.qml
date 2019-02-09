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
import QtQuick.Controls 1.4

import "../components"

Node {
    id: root

    title: "Math"
    inputs: ({x: { type: Socket.Scalar | Socket.Input, generic: true, repeating: true }})
    outputs: ({y: { type: Socket.Scalar | Socket.Output , generic: true, repeating: true }})
    hooks: QtObject { property var y: [ "x", ui.expr.text ] }

    ColumnLayout {
        property var expr: exprItem

        Text { text: "Expression:" }
        Input {
            id: exprItem

            onFocusChanged: {
                if (!focus) {
                    nodeChanged(root, ["y"])
                }
            }
        }
    }
}
