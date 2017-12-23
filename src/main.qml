import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import "main.js" as Main
import "."

ApplicationWindow {
    title: "Laph"
    visible: true
    width: Screen.width / 1.5
    height: Screen.height / 1.5
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2

    Canvas {
        id: canvas
        anchors.fill: parent

        focus: FocusSingleton.canvasFocus

        property var nodes: []
        property real xOffset: 0
        property real yOffset: 0
        property real scaling: 1
        property int nodeCount: 0
        property bool controlPressed: false

        function addNode(nodeFile, x, y) {
            var nodeComponent = Qt.createComponent(nodeFile)
            if (nodeComponent.status == Component.Ready) {
                var node = nodeComponent.createObject(canvas, {"index": nodeCount,
                                                               "xDrag": x,
                                                               "yDrag": y,
                                                               "xOffset": Qt.binding(function() { return canvas.xOffset }),
                                                               "yOffset": Qt.binding(function() { return canvas.yOffset }),
                                                               "canvas":  Qt.binding(function() { return canvas })})
                if (node != null) {
                    nodeCount += 1
                    nodes.push(node)
                } else {
                    console.error("Object could not be created")
                }
            } else {
                console.error("Component is not ready")
            }
        }

        Keys.onPressed: {
            if (event.modifiers & Qt.ControlModifier) {
                controlPressed = true
            }

            // Home key: center canvas
            if (event.key == Qt.Key_Home) {
                canvas.scaling = 1
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

            acceptedButtons: Qt.AllButtons & ~Qt.MiddleButton

            onClicked: {
                parent.focus = true
            }
        }

        MouseArea {
            anchors.fill: parent

            hoverEnabled: true
            acceptedButtons: Qt.MiddleButton

            property real oldX
            property real oldY
            property real step: 0.01

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
                        if (maxDelta < 0 && parent.scaling > 0.4) {
                            parent.scaling -= step
                        } else if (maxDelta > 0 && parent.scaling < 3) {
                            parent.scaling += step
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
            ctx.reset()
            ctx.strokeStyle = "#353535";
            ctx.fillStyle = "#494949"
            Main.drawGrid(ctx, width, height, scaling, xOffset, yOffset);
            ctx.stroke();
        }
    }
}
