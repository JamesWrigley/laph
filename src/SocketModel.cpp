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

#include <algorithm>

#include "SocketModel.hpp"

#ifdef QT_DEBUG
#include <iostream>
#endif

SocketModel::SocketModel(SocketModel const& other) : QAbstractListModel(other.parent()),
                                                     xcom(XCom::get())
{
    this->nodeIndex = other.nodeIndex;
    this->socketsType = other.socketsType;
    this->sockets = other.sockets;
    this->socket_counts = other.socket_counts;
}

SocketModel::SocketModel(SocketType type, QObject* parent) : QAbstractListModel(parent),
                                                             xcom(XCom::get())
{
    this->socketsType = type;
    connect(&xcom, &XCom::createSocket, this, &SocketModel::onCreateSocket);
    connect(&xcom, &XCom::deleteSocket, this, &SocketModel::onDeleteSocket);
}

void SocketModel::addSocket(Socket& socket, SocketConstIterator pos)
{
    auto first{pos == this->sockets.end() ?
            this->sockets.size() : pos - this->sockets.begin()};
    this->beginInsertRows(QModelIndex(), first, first);

    this->sockets.insert(pos, socket);

    this->endInsertRows();
}

void SocketModel::removeSocket(SocketConstIterator pos)
{
    auto index{pos - this->sockets.begin()};
    this->beginRemoveRows(QModelIndex(), index, index);

    this->sockets.erase(pos);

    this->endRemoveRows();
}

SocketConstIterator SocketModel::findSocket(QString const& socket_name)
{
    return std::find_if(this->sockets.cbegin(), this->sockets.cend(),
                        [&socket_name] (Socket const& socket) {
                            return socket.name == socket_name;
                        });
}

SocketIterator SocketModel::begin()
{
    return this->sockets.begin();
}

SocketIterator SocketModel::end()
{
    return this->sockets.end();
}

SocketConstIterator SocketModel::cbegin() const
{
    return this->sockets.cbegin();
}

SocketConstIterator SocketModel::cend() const
{
    return this->sockets.cend();
}

void SocketModel::setTemplate(QVariantMap const& socketTemplate)
{
    this->sockets.clear();

    auto end_it{socketTemplate.cend()};
    for (auto socket_it{socketTemplate.cbegin()}; socket_it != end_it; ++socket_it) {
        Socket socket{};
        socket.name = socket_it.key();
        socket.prefix = socket.name;

        QVariantMap properties{socket_it.value().toMap()};
        socket.type = properties.value("type").value<Socket::SocketType>();
        socket.repeating = properties.value("repeating", false).toBool();

        emit this->xcom.requestCreateSocket(socket, this->nodeIndex, this->sockets.size());
    }
}

void SocketModel::onCreateSocket(Socket socket, unsigned int nodeIndex, unsigned int socketIndex)
{
    if (nodeIndex == this->nodeIndex && ioTypesMatch(this->socketsType, socket.type)) {
        if (this->socket_counts.find(socket.prefix) == this->socket_counts.end()) {
            this->socket_counts.insert({socket.prefix, 0});
        }

        this->socket_counts.at(socket.prefix) += 1;
        this->addSocket(socket, this->cbegin() + socketIndex);
    }
}

void SocketModel::onDeleteSocket(SocketType type, unsigned int nodeIndex, unsigned int socketIndex)
{
    if (nodeIndex == this->nodeIndex && this->sockets.size() > 0 && ioTypesMatch(type, this->socketsType)) {
        auto socket_it{this->cbegin() + socketIndex};
        this->removeSocket(socket_it);
    }
}

void SocketModel::connectSocket(QString const& socket_name)
{
    auto socket_it{this->findSocket(socket_name)};

    // At least for now, we only allow repeating *input* sockets
    if (socket_it->repeating && (socket_it->type & SocketType::Input)) {
        unsigned int count{this->socket_counts.at(socket_it->prefix)};
        Socket new_socket{*socket_it};
        new_socket.name = socket_it->prefix + QString::number(count + 1);
        unsigned int socketIndex{static_cast<unsigned int>(socket_it - this->sockets.cbegin() + 1)};
        xcom.requestCreateSocket(new_socket, this->nodeIndex, socketIndex);
    }
}

void SocketModel::disconnectSocket(QString const& socket_name)
{
    auto socket_it{this->findSocket(socket_name)};

    if (socket_it->repeating) {
        auto socket_instances{std::count_if(this->sockets.cbegin(), this->sockets.cend(),
                                            [&] (Socket const& socket) {
                                                return socket.prefix == socket_it->prefix;
                                            })};
        if (socket_instances > 1) {
            unsigned int socketIndex{static_cast<unsigned int>(socket_it - this->sockets.cbegin())};
            xcom.requestDeleteSocket(*socket_it, this->nodeIndex, socketIndex);
        }
    } else {
        this->socket_counts.at(socket_it->prefix) -= 1;
    }
}

Qt::ItemFlags SocketModel::flags(QModelIndex const&) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

int SocketModel::rowCount(QModelIndex const&) const
{
    return this->sockets.size();
}

QVariant SocketModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid() || index.row() >= (int)this->sockets.size()) {
        return QVariant{};
    }

    auto& socket{this->sockets.at(index.row())};
    if (role == NameRole) {
        return socket.name;
    } else if (role == TypeRole) {
        return socket.type;
    } else if (role == RepeatingRole) {
        return socket.repeating;
    } else {
        return QVariant{};
    }
}

bool SocketModel::setData(QModelIndex const& index, QVariant const& value, int role)
{
    if (!index.isValid() || index.row() >= (int)this->sockets.size()) {
        return false;
    }

    auto& socket{this->sockets.at(index.row())};
    if (role == NameRole) {
        socket.name = value.toString();
    } else if (role == TypeRole) {
        socket.type = value.value<Socket::SocketType>();
    } else if (role == RepeatingRole) {
        socket.repeating = value.toBool();
    } else {
        return false;
    }

    emit this->dataChanged(index, index, {role});
    return true;
}

QHash<int, QByteArray> SocketModel::roleNames() const
{
    return QHash<int, QByteArray>{{NameRole, "name"},
                                  {TypeRole, "type"},
                                  {RepeatingRole, "repeating"}};
}
