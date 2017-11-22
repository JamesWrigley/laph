import QtQuick 2.7
import QtQuick.Controls 1.4

ApplicationWindow {
    title: "Laph"
    width: 640
    height: 480
    visible: true

    Rectangle {
        id: canvas
        anchors.fill: parent

        color: "#494949"

        Canvas {
            focus: true
            anchors.fill: parent

            property real xOffset: 0
            property real yOffset: 0

            function drawGrid(ctx) {
                var sep = 50;

                function transform(pos, offset, dimension) {
                    var new_pos = pos + offset
                    var cd = dimension + ((sep - dimension % sep)) % sep;

                    if (new_pos >= cd) {
                        new_pos %= cd
                    } else if (new_pos < 0) {
                        new_pos = cd - (Math.abs(new_pos) % cd);
                    }

                    return new_pos;
                }

                for (var i = 0; i < Math.max(width, height); i += sep) {
                    var x = transform(i, xOffset, width);
                    var y = transform(i, yOffset, height);

                    ctx.moveTo(x, 0);
                    ctx.lineTo(x, height);
                    ctx.moveTo(0, y);
                    ctx.lineTo(width, y);
                }
            }

            Keys.onPressed: {
                if (event.key == Qt.Key_Home) {
                    xOffset = 0
                    yOffset = 0
                    requestPaint()
                } else if (event.key == Qt.Key_Q &&
                           event.modifiers & Qt.ControlModifier) {
                    Qt.quit()
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                property real oldX
                property real oldY

                acceptedButtons: Qt.MiddleButton
                onPositionChanged: {
                    if (pressed) {
                        parent.xOffset += mouse.x - oldX
                        parent.yOffset += mouse.y - oldY
                        parent.requestPaint()
                    }

                    oldX = mouse.x
                    oldY = mouse.y
                }
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = "#353535";
                drawGrid(ctx);
                ctx.stroke();
            }
        }
    }
}
