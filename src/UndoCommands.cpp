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
    WireItem const* wire{this->glaph.findWire(this->inputIndex, this->inputSocket,
                                              this->outputIndex, this->outputSocket)};
    emit this->xcom.deleteWire(wire->index);
}

void WireCommand::createWire()
{
    unsigned int startIndex{this->startIsInput ? this->inputIndex : this->outputIndex};
    QString& startSocket{this->startIsInput ? this->inputSocket : this->outputSocket};

    NodeItem* startNode{this->glaph.getNode(startIndex)};
    auto getObjName{[&] (QVariant& return_var, QString name, QString& socket, bool socketIsInput) {
                        QMetaObject::invokeMethod(startNode, "getObjectName",
                                                  Q_RETURN_ARG(QVariant, return_var),
                                                  Q_ARG(QVariant, name),
                                                  Q_ARG(QVariant, socket),
                                                  // Need to flip the bool here because the argument means if the
                                                  // socket is an input, not the wire tip.
                                                  Q_ARG(QVariant, socketIsInput));
                    }};
    QVariant maName{};
    QVariant daName{};
    getObjName(maName, "ma", startSocket, !this->startIsInput);
    getObjName(daName, "da", startSocket, !this->startIsInput);
    QObject* ma{findChildItem(startNode, maName.toString())};
    QObject* da{findChildItem(startNode, daName.toString())};
    QVariant wire;
    QMetaObject::invokeMethod(startNode, "createWire",
                              Q_RETURN_ARG(QVariant, wire),
                              Q_ARG(QVariant, QVariant::fromValue(da)),
                              Q_ARG(QVariant, QVariant::fromValue(ma)),
                              Q_ARG(QVariant, this->startIsInput));
    WireItem* wire_ptr{wire.value<WireItem*>()};
    wire_ptr->creationCommand = this;

    // If the wire has been properly connected between two nodes, we need to
    // connect it again to the same socket.
    if (this->connected) {
        unsigned int endIndex{this->startIsInput ? this->outputIndex : this->inputIndex};
        QString& endSocket{this->startIsInput ? this->outputSocket : this->inputSocket};
        QObject* endTip{wire_ptr->property("endTip").value<QObject*>()};
        NodeItem* endNode{this->glaph.getNode(endIndex)};

        QVariant endDaName{};
        getObjName(endDaName, "da", endSocket, this->startIsInput);
        QObject* endDa{findChildItem(static_cast<NodeItem*>(endNode), endDaName.toString())};
        println("index: :2, :0: :1", {endDaName.toString(), (unsigned long long)endDa, static_cast<NodeItem*>(endNode)->index});
        endTip->setProperty("Drag.target", QVariant::fromValue(endDa));

        QMetaObject::invokeMethod(wire_ptr, "handleRelease", Q_ARG(QVariant, QVariant::fromValue(endTip)));
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

DeleteWire::DeleteWire(Glaph& glaph, unsigned int inputIndex, QString const& inputSocket,
                       unsigned int outputIndex, QString const& outputSocket) : WireCommand(glaph)
{
    this->inputIndex = inputIndex;
    this->inputSocket = inputSocket;
    this->outputIndex = outputIndex;
    this->outputSocket = outputSocket;
    this->connected = true;
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
