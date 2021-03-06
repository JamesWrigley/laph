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
#include <unordered_map>

#include <QHash>
#include <QByteArray>
#include <QModelIndex>
#include <QVariantMap>
#include <QAbstractListModel>

#include "util.hpp"
#include "XCom.hpp"
#include "Socket.hpp"

using SocketVector = std::vector<Socket>;
using SocketIterator = SocketVector::iterator;
using SocketConstIterator = SocketVector::const_iterator;

class SocketModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SocketModel(SocketModel const&);
    SocketModel(SocketType = SocketType::Input, QObject* = Q_NULLPTR);

    enum SocketRoles { NameRole = Qt::UserRole + 1,
                       TypeRole,
                       RepeatingRole };

    void connectSocket(QString const&);
    void disconnectSocket(QString const&);
    void setTemplate(QVariantMap const&);

    SocketIterator begin();
    SocketIterator end();
    SocketConstIterator cbegin() const;
    SocketConstIterator cend() const;

    unsigned int nodeIndex;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void addSocket(Socket&, SocketConstIterator);
    void removeSocket(SocketConstIterator);
    SocketConstIterator findSocket(QString const&);

    Qt::ItemFlags flags(QModelIndex const&) const;
    int rowCount(QModelIndex const& = QModelIndex()) const;
    QVariant data(QModelIndex const&, int = Qt::DisplayRole) const;
    bool setData(QModelIndex const&, QVariant const&, int = Qt::EditRole);

    XCom& xcom;
    SocketType socketsType;
    SocketVector sockets{};
    std::unordered_map<QString, unsigned int> socket_counts{};

private slots:
    void onCreateSocket(Socket, unsigned int nodeIndex, unsigned int socketIndex);
    void onDeleteSocket(SocketType, unsigned int nodeIndex, unsigned int socketIndex);
};

#endif
