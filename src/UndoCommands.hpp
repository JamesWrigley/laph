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

class NodeCommand : public QUndoCommand
{
public:
    NodeCommand();

    virtual void undo() = 0;
    virtual void redo() = 0;

protected:
    XCom& xcom;
    int x;
    int y;
    int index;
    QString nodeFile;
};

class CreateNode : public NodeCommand
{
public:
    CreateNode(QString const&, int, int, int);

    void undo() override;
    void redo() override;
};

class DeleteNode : public NodeCommand
{
public:
    DeleteNode(NodeItem const*);

    void undo() override;
    void redo() override;
};

#endif
