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

#include <QQmlProperty>

#include "WireItem.hpp"
#include "UndoCommands.hpp"

WireItem::WireItem(QQuickItem* parent) : QQuickItem(parent)
{
    static int wire_count{0};

    this->index = wire_count;
    emit this->indexChanged();
    ++wire_count;
}

WireItem::~WireItem()
{
    // If the wire is new, we should make its creation command obsolete so that
    // it doesn't execute when the user hits undo.
    if (this->isNewSocket()) {
        this->creationCommand->setObsolete(true);
    }
}

bool WireItem::isNewSocket()
{
    return this->endParent == this->initialSocket;
}

int WireItem::getIndex()
{
    return this->index;
}

bool WireItem::getValid()
{
    return this->valid;
}

QObject* WireItem::getEndParent()
{
    return this->endParent;
}

QString WireItem::getInputSocket()
{
    return this->inputSocket;
}

QString WireItem::getOutputSocket()
{
    return this->outputSocket;
}

QObject* WireItem::getInitialSocket()
{
    return this->initialSocket;
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

void WireItem::setEndParent(QObject* endParent)
{
    QObject* initialSocket{this->property("initialSocket").value<QObject*>()};
    if (endParent != nullptr && initialSocket != nullptr &&
        this->endParent != endParent && this->endParent == initialSocket) {
        QObject* endTip{this->property("endTip").value<QObject*>()};
        this->creationCommand->setEndProperties(endTip->property("index").toInt(),
                                                endTip->property("socketName").toString());
    }

    this->endParent = endParent;
    emit this->endParentChanged();
}

void WireItem::setInputSocket(QString& socketName)
{
    if (&socketName != &(this->inputSocket)) {
        this->inputSocket = socketName;
        emit this->inputSocketChanged();
    }
}

void WireItem::setOutputSocket(QString& socketName)
{
    if (&socketName != &(this->outputSocket)) {
        this->outputSocket = socketName;
        emit this->outputSocketChanged();
    }
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

void WireItem::setInitialSocket(QObject* socket)
{
    this->initialSocket = socket;
    emit this->initialSocketChanged();
}
