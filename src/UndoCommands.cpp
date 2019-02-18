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

/*** CreateNode ***/

CreateNode::CreateNode(QString const& nodeFile, int index, int x, int y)
{
    this->x = x;
    this->y = y;
    this->index = index;
    this->nodeFile = nodeFile;
}

void CreateNode::undo()
{
    emit this->xcom.deleteNode(this->index);
}

void CreateNode::redo()
{
    emit this->xcom.createNode(this->nodeFile, this->index, this->x, this->y);
}

/*** DeleteNode ***/

DeleteNode::DeleteNode(NodeItem const* node)
{
    this->x = node->x();
    this->y = node->y();
    this->index = node->index;
    this->nodeFile = node->nodeFile;
}

void DeleteNode::undo()
{
    emit this->xcom.createNode(this->nodeFile, this->index, this->x, this->y);
}

void DeleteNode::redo()
{
    emit this->xcom.deleteNode(this->index);
}

/*** WireCommand ***/

WireCommand::WireCommand(Glaph& glaph) : xcom(XCom::get()), glaph(glaph) { }

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

void CreateWire::undo()
{
    WireItem const* wire{this->glaph.findWire(this->inputIndex, this->inputSocket,
                                              this->outputIndex, this->outputSocket)};
    emit this->xcom.deleteWire(wire->index);
}

void CreateWire::redo()
{
    unsigned int startIndex{this->startIsInput ? this->inputIndex : this->outputIndex};
    QString& startSocket{this->startIsInput ? this->inputSocket : this->outputSocket};

    NodeItem* startNode{this->glaph.getNode(startIndex)};
    auto find_obj{[&] (QVariant& return_var, QString name) {
                      QMetaObject::invokeMethod(startNode, "getObjectName",
                                                Q_RETURN_ARG(QVariant, return_var),
                                                Q_ARG(QVariant, name),
                                                Q_ARG(QVariant, startSocket),
                                                // Need to flip the bool here because the argument means if the
                                                // socket is an input, not the wire tip.
                                                Q_ARG(QVariant, !this->startIsInput));
                  }};
    QVariant maName{};
    QVariant daName{};
    find_obj(maName, "ma");
    find_obj(daName, "da");
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
}

void CreateWire::setEndProperties(unsigned int index, QString const& socketName)
{
    if (this->startIsInput) {
        this->outputIndex = index;
        this->outputSocket = socketName;
    } else {
        this->inputIndex = index;
        this->inputSocket = socketName;
    }
}

/*** DeleteWire ***/

DeleteWire::DeleteWire(Glaph& glaph, unsigned int inputIndex, QString const& inputSocket,
                       unsigned int outputIndex, QString const& outputSocket) : WireCommand(glaph)
{
    this->inputIndex = inputIndex;
    this->inputSocket = inputSocket;
    this->outputIndex = outputIndex;
    this->outputSocket = outputSocket;
}

void DeleteWire::undo() { }

void DeleteWire::redo() { }
