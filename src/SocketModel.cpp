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

SocketModel::SocketModel(SocketModel const& other) : SocketModel(other.parent(),
                                                                 other.socketsTemplate,
                                                                 other.sockets) { }

SocketModel::SocketModel(QObject* parent,
                         QVariantMap socketsTemplate,
                         SocketVector sockets) : QAbstractListModel(parent)
{
    this->socketsTemplate = socketsTemplate;
    this->sockets = sockets;

    connect(this, &SocketModel::socketsTemplateChanged,
            this, &SocketModel::refreshSockets);
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

SocketConstIterator SocketModel::cbegin() const
{
    return this->sockets.cbegin();
}

SocketConstIterator SocketModel::cend() const
{
    return this->sockets.cend();
}

void SocketModel::refreshSockets()
{
    this->sockets.clear();

    auto end_it{this->socketsTemplate.end()};
    for (auto socket_it{this->socketsTemplate.begin()}; socket_it != end_it; ++socket_it) {
        Socket socket{};
        socket.name = socket_it.key();
        socket.prefix = socket.name;

        QVariantMap properties{socket_it.value().toMap()};
        socket.type = properties.value("type").value<Socket::SocketType>();
        socket.generic = properties.value("generic", false).toBool();
        socket.repeating = properties.value("repeating", false).toBool();

        this->addSocket(socket, this->sockets.end());
        this->socket_counts.insert({socket.prefix, 1});
    }
}

void SocketModel::connectSocket(QString const& socket_name)
{
    auto socket_it{this->findSocket(socket_name)};
    auto dist{std::distance(this->cbegin(), socket_it)};
    std::cout << "Connecting " << socket_name << " " << dist << "\n";
    unsigned int& count{this->socket_counts.at(socket_it->prefix)};
    ++count;

    if (socket_it->repeating) {
        Socket new_socket{*socket_it};
        new_socket.name = socket_it->prefix + QString::number(count);
        this->addSocket(new_socket, socket_it + 1);
    }
}

void SocketModel::disconnectSocket(QString const& socket_name)
{
    auto socket_it{this->findSocket(socket_name)};
    if (socket_it->repeating) {
        unsigned int count{this->socket_counts.at(socket_it->prefix)};

        if (count > 1) {
            this->removeSocket(socket_it);
        }
    } else {
        this->socket_counts.at(socket_it->prefix) -= 1;
    }
}

QVariantMap SocketModel::getSocketsTemplate() { return this->socketsTemplate; }

void SocketModel::setSocketsTemplate(QVariantMap const& socketsTemplate)
{
    this->socketsTemplate = socketsTemplate;
    emit this->socketsTemplateChanged();
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
    } else if (role == GenericRole) {
        return socket.generic;
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
    } else if (role == GenericRole) {
        socket.generic = value.toBool();
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
                                  {GenericRole, "generic"},
                                  {RepeatingRole, "repeating"}};
}
