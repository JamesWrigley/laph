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
