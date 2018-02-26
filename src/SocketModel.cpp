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

#include "SocketModel.hpp"

SocketModel::SocketModel(QObject* parent) : QAbstractListModel(parent) { }

bool SocketModel::addSocket(Socket& socket, int index)
{
    int first{index == -1 ? (int)this->sockets.size() : index};
    this->beginInsertRows(QModelIndex(), first, first);

    bool result{true};
    if (index == -1) {
        this->sockets.push_back(socket);
    } else if (index >= 0 && index < (int)this->sockets.size()) {
        this->sockets.insert(this->sockets.begin() + index, socket);
    } else {
        result = false;
    }

    this->endInsertRows();
    return result;
}

bool SocketModel::removeSocket(int index)
{
    this->beginRemoveRows(QModelIndex(), index, index);

    bool result{false};
    if (index >= 0 && index < (int)this->sockets.size()) {
        this->sockets.erase(this->sockets.begin() + index);
        result = true;
    }

    this->endRemoveRows();
    return result;
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
