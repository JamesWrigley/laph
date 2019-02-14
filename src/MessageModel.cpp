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

#include "MessageModel.hpp"

MessageModel::MessageModel(QObject* parent) : QAbstractListModel(parent) { }

void MessageModel::addMessage(QString const& msg, MessageLevel level)
{
    this->beginInsertRows(QModelIndex(), 0, 0);

    this->msgs.insert(this->msgs.begin(), {msg, level});

    this->endInsertRows();
}

void MessageModel::clearMessages()
{
    this->beginRemoveRows(QModelIndex(), 0, this->msgs.size() - 1);

    this->msgs.clear();

    this->endRemoveRows();
}

Qt::ItemFlags MessageModel::flags(QModelIndex const&) const
{
    return Qt::ItemIsEnabled;
}

int MessageModel::rowCount(QModelIndex const&) const
{
    return this->msgs.size();
}

QVariant MessageModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid() || index.row() >= (int)this->msgs.size()) {
        return QVariant();
    }

    std::pair<QString, MessageLevel> const& msg{this->msgs.at(index.row())};
    if (role == MsgRole) {
        return msg.first;
    } else if (role == LevelRole) {
        return msg.second;
    } else {
        return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return QHash<int, QByteArray>{{MsgRole, "msg"},
                                  {LevelRole, "level"}};
}
