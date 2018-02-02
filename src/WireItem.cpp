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

#include <cstdint>

#include "WireItem.hpp"

WireItem::WireItem(QQuickItem* parent) : QQuickItem(parent) { }

QString WireItem::getInputSocket()
{
    return this->inputSocket;
}

QString WireItem::getOutputSocket()
{
    return this->outputSocket;
}

QQuickItem* WireItem::getInputNode()
{
    return static_cast<QQuickItem*>(this->inputNode);
}

QQuickItem* WireItem::getOutputNode()
{
    return static_cast<QQuickItem*>(this->outputNode);
}

void WireItem::setInputSocket(QString& socketName)
{
    this->inputSocket = socketName;
    emit this->inputSocketChanged();
}

void WireItem::setOutputSocket(QString& socketName)
{
    this->outputSocket = socketName;
    emit this->outputSocketChanged();
}

void WireItem::setInputNode(QQuickItem* node)
{
    if (node != nullptr) {
        this->inputNode = static_cast<NodeItem*>(node);
        emit this->inputNodeChanged();
    }
}

void WireItem::setOutputNode(QQuickItem* node)
{
    if (node != nullptr) {
        this->outputNode = static_cast<NodeItem*>(node);
        emit this->outputNodeChanged();
    }
}
