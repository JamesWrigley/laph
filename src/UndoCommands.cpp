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

#include "util.hpp"
#include "Glaph.hpp"
#include "UndoCommands.hpp"

/*** NodeCommand ***/

NodeCommand::NodeCommand() : xcom(XCom::get()) { }

void NodeCommand::deleteNode()
{
    emit this->xcom.deleteNode(this->index);
}

void NodeCommand::createNode()
{
    emit this->xcom.createNode(this->nodeFile, this->index, this->x, this->y);
}

/*** CreateNode ***/

CreateNode::CreateNode(QString const& nodeFile, int index, int x, int y)
{
    this->x = x;
    this->y = y;
    this->index = index;
    this->nodeFile = nodeFile;
}

void CreateNode::undo() { this->deleteNode(); }

void CreateNode::redo() { this->createNode(); }

/*** DeleteNode ***/

DeleteNode::DeleteNode(NodeItem const* node)
{
    this->x = node->x();
    this->y = node->y();
    this->index = node->index;
    this->nodeFile = node->nodeFile;
}

void DeleteNode::undo() { this->createNode(); }

void DeleteNode::redo() { this->deleteNode(); }

/*** WireCommand ***/

WireCommand::WireCommand(Glaph& glaph) : xcom(XCom::get()), glaph(glaph) { }

void WireCommand::deleteWire()
{
    emit this->xcom.deleteWire(this->wireIndex);
}

void WireCommand::createWire()
{
    unsigned int startIndex{this->startIsInput ? this->inputIndex : this->outputIndex};
    QString& startSocket{this->startIsInput ? this->inputSocket : this->outputSocket};

    NodeItem* startNode{this->glaph.getNode(startIndex)};
    QString maName{getObjectName(startNode, "ma", startSocket, !this->startIsInput)};
    QString daName{getObjectName(startNode, "da", startSocket, !this->startIsInput)};
    QObject* ma{findChildItem(startNode, maName)};
    QObject* da{findChildItem(startNode, daName)};
    QVariant wire;
    QMetaObject::invokeMethod(startNode, "createWire",
                              Q_RETURN_ARG(QVariant, wire),
                              Q_ARG(QVariant, QVariant::fromValue(da)),
                              Q_ARG(QVariant, QVariant::fromValue(ma)),
                              Q_ARG(QVariant, this->startIsInput));
    WireItem* wire_ptr{wire.value<WireItem*>()};
    wire_ptr->creationCommand = this;

    // If it is the first time we are creating this particular wire, store its
    // index so that we can set it correctly if the wire recreated later
    // (i.e. via some undo/redo).
    if (this->wireIndex == -1) {
        this->wireIndex = wire_ptr->index;
    } else {
        wire_ptr->setIndex(this->wireIndex);
    }

    this->glaph.addWire(wire_ptr);

    // If the wire has been properly connected between two nodes, we need to
    // connect it again to the same socket.
    if (this->connected) {
        unsigned int endIndex{this->startIsInput ? this->outputIndex : this->inputIndex};
        QString& endSocket{this->startIsInput ? this->outputSocket : this->inputSocket};
        QObject* endTip{wire_ptr->property("endTip").value<QObject*>()};
        NodeItem* endNode{this->glaph.getNode(endIndex)};

        QString endDaName{getObjectName(endNode, "da", endSocket, this->startIsInput)};
        QObject* endDa{findChildItem(static_cast<NodeItem*>(endNode), endDaName)};
        emit this->xcom.connectWireTip(endTip, endDa, XCom::ConnectionType::New);
    }
}

void WireCommand::setEndProperties(unsigned int index, QString const& socketName)
{
    if (this->startIsInput) {
        this->outputIndex = index;
        this->outputSocket = socketName;
    } else {
        this->inputIndex = index;
        this->inputSocket = socketName;
    }

    this->connected = true;
}

/*** CreateWire ***/

CreateWire::CreateWire(Glaph& glaph, unsigned int startIndex,
                       QString const& startSocket, bool startSocketIsInput) : WireCommand(glaph)
{
    // If it's an output socket, then it'll be connected to an input wire tip, and vice versa
    this->startIsInput = !startSocketIsInput;

    if (this->startIsInput) {
        this->inputIndex = startIndex;
        this->inputSocket = startSocket;
    } else {
        this->outputIndex = startIndex;
        this->outputSocket = startSocket;
    }
}

void CreateWire::undo() { this->deleteWire(); }

void CreateWire::redo() { this->createWire(); }

/*** DeleteWire ***/

DeleteWire::DeleteWire(Glaph& glaph, int wireIndex) : WireCommand(glaph)
{
    WireItem const* wire{this->glaph.getWire(wireIndex)};

    this->inputIndex = wire->inputNode->index;
    this->inputSocket = wire->inputSocket;
    this->outputIndex = wire->outputNode->index;
    this->outputSocket = wire->outputSocket;
    this->connected = true;

    QObject* endTip{wire->property("endTip").value<QObject*>()};
    this->startIsInput = endTip->property("socketName").toString() != inputSocket;
    this->wireIndex = wire->index;
}

void DeleteWire::undo() { this->createWire(); }

void DeleteWire::redo() { this->deleteWire(); }

/*** SocketCommand ***/

SocketCommand::SocketCommand(Socket const& socket, unsigned int nodeIndex, unsigned int socketIndex) : xcom(XCom::get()),
                                                                                                       socket(socket),
                                                                                                       nodeIndex(nodeIndex),
                                                                                                       socketIndex(socketIndex)
{ }

void SocketCommand::createSocket()
{
    xcom.createSocket(this->socket, this->nodeIndex, this->socketIndex);
}

void SocketCommand::deleteSocket()
{
    xcom.deleteSocket(this->socket.type, this->nodeIndex, this->socketIndex);
}

/*** CreateSocket ***/

void CreateSocket::undo() { this->deleteSocket(); }

void CreateSocket::redo() { this->createSocket(); }

/*** DeleteSocket ***/

void DeleteSocket::undo() { this->createSocket(); }

void DeleteSocket::redo() { this->deleteSocket(); }
