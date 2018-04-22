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

#ifndef QT_DEBUG
#include <iostream>

#include "util.hpp"
#endif

#include "WireItem.hpp"

WireItem::WireItem(QQuickItem* parent) : QQuickItem(parent)
{
    static int wire_count{0};

    this->index = wire_count;
    emit this->indexChanged();
    ++wire_count;

    auto printer{[this] () {
            if (this->inputSocket != nullptr && this->outputSocket != nullptr) {
                std::cout << "inputSocket: " << this->inputSocket
                          << ", outputSocket: " << this->outputSocket << "\n";
            }
        }};
    connect(this, &WireItem::inputSocketChanged, printer);
    connect(this, &WireItem::outputSocketChanged, printer);
}

int WireItem::getIndex()
{
    return this->index;
}

bool WireItem::getValid()
{
    return this->valid;
}

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
    return reinterpret_cast<QQuickItem*>(this->inputNode);
}

QQuickItem* WireItem::getOutputNode()
{
    return reinterpret_cast<QQuickItem*>(this->outputNode);
}

void WireItem::setValid(bool valid)
{
    this->valid = valid;
    emit this->validChanged();
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
    this->inputNode = reinterpret_cast<NodeItem*>(node);
    emit this->inputNodeChanged();
}

void WireItem::setOutputNode(QQuickItem* node)
{
    this->outputNode = reinterpret_cast<NodeItem*>(node);
    emit this->outputNodeChanged();
}
