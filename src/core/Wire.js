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

function evaluateInput() {
    if (valid) {
        graphEngine.evaluateFrom(inputNode, [inputSocket])
    }
}

function setParent(wireTip) {
    var target = wireTip.Drag.target
    wireTip.index = target.node.index

    var hook = Qt.binding(function () {
        return target.node === null ? 0 : target.node.x + target.node.y + canvas.scaling + target.socketChangeHook
    })

    if (wireTip === start.item) {
        root.startUpdateHook = hook
    } else {
        root.endUpdateHook = hook
    }

    // Change these after setting the hook so that the wire coordinates will
    // be recalculated.
    wireTip.parent = target
    wireTip.x = 0
    wireTip.y = 0
}

function handleReleaseImpl(wireTip) {
    var target = wireTip.Drag.target
    var otherTip = wireTip === endTip ? start.item : endTip

    // When connecting a wire
    if (target !== null && wireTip.Drag.drop() === Qt.MoveAction) {
        if (target.isInput) {
            root.inputNode = target.node
            root.inputSocket = target.socketName
            root.outputNode = graphEngine.getNode(otherTip.index)
            root.outputSocket = otherTip.socketName
        } else {
            root.outputNode = target.node
            root.outputSocket = target.socketName
            root.inputNode = graphEngine.getNode(otherTip.index)
            root.inputSocket = otherTip.socketName
        }

        // If the wire has just been created
        if (endParent === initialSocket) {
            xcom.wireConnected(target.node.index,
                               wireTip.isOutput ? XCom.Output : XCom.Input,
                               target.socketName)

            xcom.wireConnected(otherTip.index,
                               wireTip.isOutput ? XCom.Input : XCom.Output,
                               otherTip.socketName)
        } else if (wireTip.parent === target) {
            // If the user is reconnecting to the same socket, all we need to do
            // is reset the wireTip's local coordinates.
            wireTip.x = 0
            wireTip.y = 0
            return
        } else {
            // If connecting to a different socket
            xcom.wireDisconnected(wireTip.index, wireTip.isOutput ? XCom.Output : XCom.Input, wireTip.socketName)
            xcom.wireConnected(target.node.index, wireTip.isOutput ? XCom.Output : XCom.Input, target.socketName)
        }

        // If the socket already has a wire connected, disconnect it
        if (!target.isInput && target.wires > 0) {
            var extantWireTip = target.children[0]
            graphEngine.removeWire(extantWireTip.wireIndex)
        }

        setParent(wireTip)
        evaluateInput()
    } else { // When disconnecting a wire
        xcom.repaintCanvas()
        graphEngine.removeWire(root.index)
    }
}

function computeCoord(wireTip, hook, x) {
    if (wireTip.parent !== null) {
        var oldCoord = (x ? wireTip.x : wireTip.y) + wireTip.width / 2
        var newCoord = (wireTip.Drag.active ? wireTip.parent : wireTip)
            .mapToItem(root.canvas, x ? oldCoord : hook, x ? hook : oldCoord)

        return x ? newCoord.x : newCoord.y
    } else {
        return 0
    }
}
