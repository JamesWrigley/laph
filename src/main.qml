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

            property real scale: 1
            property real xOffset: 0
            property real yOffset: 0
            property bool controlPressed: false

            function drawGrid(ctx) {
                var sep = 50 * scale;

                function transform(pos, offset, dimension) {
                    var new_pos = pos + offset
                    var cd = dimension + ((sep - dimension % sep)) % sep;

                    if (new_pos >= cd) {
                        new_pos %= cd;
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
                if (event.modifiers & Qt.ControlModifier) {
                    controlPressed = true
                }

                // Home key: center canvas
                if (event.key == Qt.Key_Home) {
                    scale = 1
                    xOffset = 0
                    yOffset = 0
                    requestPaint()
                } else if (event.key == Qt.Key_Q &&
                           event.modifiers & Qt.ControlModifier) { // Ctrl + Q: Quit
                        Qt.quit()
                }
            }

            Keys.onReleased: {
                if (event.key & Qt.Key_Control) {
                    controlPressed = false
                }
            }

            // Handle MMB drags
            MouseArea {
                anchors.fill: parent

                property real oldX
                property real oldY

                hoverEnabled: true
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

            // Handle wheel events, which are used for translation unless the
            // Ctrl modifier is also pressed.
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.AllButtons

                property real oldX
                property real oldY
                property real step: 0.04

                onWheel: {
                    if (parent.controlPressed) {
                        var delta = wheel.angleDelta
                        var maxDelta = Math.abs(delta.x) > Math.abs(delta.y) ? delta.x : delta.y

                        if (Math.abs(maxDelta) > 0) {
                            if (maxDelta < 0 && parent.scale > 0.25) {
                                parent.scale -= step
                            } else if (maxDelta > 0 && parent.scale < 3) {
                                parent.scale += step
                            }
                        }
                    } else {
                        parent.xOffset += wheel.pixelDelta.x - oldX
                        parent.yOffset += wheel.pixelDelta.y - oldY
                    }

                    parent.requestPaint()
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
