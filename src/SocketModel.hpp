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

#ifndef SOCKETMODEL_HPP
#define SOCKETMODEL_HPP

#include <vector>

#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QModelIndex>
#include <QAbstractListModel>

#include "Socket.hpp"

class SocketModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SocketModel(QObject* = Q_NULLPTR);

    enum SocketRoles { NameRole = Qt::UserRole + 1, TypeRole, GenericRole, RepeatingRole };

    bool addSocket(Socket&, int = -1);
    bool removeSocket(int);

    Qt::ItemFlags flags(QModelIndex const&) const;
    int rowCount(QModelIndex const& = QModelIndex()) const;
    QVariant data(QModelIndex const&, int = Qt::DisplayRole) const;
    bool setData(QModelIndex const&, QVariant const&, int = Qt::EditRole);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    std::vector<Socket> sockets;
};

#endif
