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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import "../core"

TextField {
    id: root

    menu: null
    font: mono.name
    style: TextFieldStyle {
        renderType: Text.QtRendering
        selectionColor: Qt.darker(root.bgColor, 2)
        background: Rectangle {
            radius: root.radius
            color: root.hovered ? Qt.lighter(root.bgColor, 1.1) : root.bgColor
        }
    }

    property var radius: 0
    property var bgColor: "lightgray"

    onEditingFinished: {
        root.focus = false
    }

    Keys.onEscapePressed: {
        root.focus = false
    }

    FontLoader {
        id: mono
        source: "../fonts/FiraMono-Regular.otf"
    }
}
