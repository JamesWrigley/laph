import QtQuick 2.7
import QtQuick.Controls 1.4
import "main.js" as Main

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
            anchors.fill: parent

            focus: true

            property real scale: 1
            property real xOffset: 0
            property real yOffset: 0
            property bool controlPressed: false

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

            MouseArea {
                anchors.fill: parent

                hoverEnabled: true
                acceptedButtons: Qt.MiddleButton

                property real oldX
                property real oldY
                property real step: 0.04

                onPositionChanged: {
                    if (pressed) {
                        parent.xOffset += mouse.x - oldX
                        parent.yOffset += mouse.y - oldY
                        parent.requestPaint()
                    }

                    oldX = mouse.x
                    oldY = mouse.y
                }

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
                        parent.xOffset += wheel.pixelDelta.x
                        parent.yOffset += wheel.pixelDelta.y
                    }

                    parent.requestPaint()
                }
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = "#353535";
                Main.drawGrid(ctx, width, height,
                              scale, xOffset, yOffset);
                ctx.stroke();
            }
        }
    }
}
