import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

TextField {
    id: root

    font: mono.name
    style: TextFieldStyle {
        selectionColor: Qt.darker(root.bgColor, 2)
        background: Rectangle {
            radius: 2
            color: root.hovered ? Qt.lighter(root.bgColor, 1.1) : root.bgColor
        }
    }

    property var bgColor: "lightgray"

    onFocusChanged: {
        FocusSingleton.canvasFocus = !focus
    }

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
