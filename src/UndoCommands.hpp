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

#ifndef UNDOCOMMANDS_HPP
#define UNDOCOMMANDS_HPP

#include <QUndoCommand>

#include "XCom.hpp"
#include "NodeItem.hpp"
#include "SocketModel.hpp"

class Glaph;

class BaseCommand : public QUndoCommand
{
public:
    BaseCommand();

    XCom& xcom;
    unsigned int const eventId;

    virtual void undo() = 0;
    virtual void redo() = 0;
};

class MoveNode : public BaseCommand
{
public:
    MoveNode(Glaph&, unsigned int, int, int, int, int);

    void undo() override;
    void redo() override;

    int id() const override;
    bool mergeWith(QUndoCommand const*) override;

private:
    Glaph& glaph;

    int oldX;
    int oldY;
    int newX;
    int newY;
    unsigned int nodeIndex;
};

class NodeCommand : public BaseCommand
{
public:
    NodeCommand(Glaph&);

    virtual void undo() = 0;
    virtual void redo() = 0;

protected:
    void createNode();
    void deleteNode();

    Glaph& glaph;
    int x;
    int y;
    int index;
    QString nodeFile;
};

class CreateNode : public NodeCommand
{
public:
    CreateNode(Glaph&, QString const&, int, int, int);

    void undo() override;
    void redo() override;
};

class DeleteNode : public NodeCommand
{
public:
    DeleteNode(Glaph&, NodeItem const*);

    void undo() override;
    void redo() override;
};

class WireCommand : public BaseCommand
{
public:
    WireCommand(Glaph&);

    virtual void undo() = 0;
    virtual void redo() = 0;

    void setEndProperties(unsigned int, QString const&);

protected:
    void createWire();
    void deleteWire();

    Glaph& glaph;

    int wireIndex{-1};
    bool startIsInput;
    bool connected{false};
    unsigned int inputIndex;
    QString inputSocket;
    unsigned int outputIndex;
    QString outputSocket;
};

class CreateWire : public WireCommand
{
public:
    CreateWire(Glaph&, unsigned int, QString const&, bool);

    void undo() override;
    void redo() override;
};

class DeleteWire : public WireCommand
{
public:
    DeleteWire(Glaph&, int);

    void undo() override;
    void redo() override;
};

class ReconnectWireTip : public BaseCommand
{
public:
    ReconnectWireTip(Glaph&, unsigned int, TipType, unsigned int, QString const&);

    void undo() override;
    void redo() override;

private:
    void reconnect(unsigned int, unsigned int, QString const&);

    Glaph& glaph;

    TipType tipType;
    bool isReplay{false};
    unsigned int wireIndex;
    QString oldSocket;
    QString newSocket;
    unsigned int oldNodeIndex;
    unsigned int newNodeIndex;
};

class SocketCommand : public BaseCommand
{
public:
    SocketCommand(Socket const&, unsigned int, unsigned int);

    void undo() = 0;
    void redo() = 0;

protected:
    void createSocket();
    void deleteSocket();

    Socket const socket;
    unsigned int nodeIndex;
    unsigned int socketIndex;
};

class CreateSocket : public SocketCommand
{
public:
    using SocketCommand::SocketCommand;

    void undo() override;
    void redo() override;
};

class DeleteSocket : public SocketCommand
{
public:
    using SocketCommand::SocketCommand;

    void undo() override;
    void redo() override;
};

#endif
