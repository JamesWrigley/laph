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
#include "Socket.hpp"

using SocketVector = std::vector<Socket>;
using SocketIterator = SocketVector::iterator;
using SocketConstIterator = SocketVector::const_iterator;

class SocketModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap socketsTemplate READ getSocketsTemplate WRITE setSocketsTemplate NOTIFY socketsTemplateChanged)

public:
    SocketModel(SocketModel const&);
    SocketModel(QObject* = Q_NULLPTR,
                QVariantMap = { },
                SocketVector = { });

    enum SocketRoles { NameRole = Qt::UserRole + 1,
                       TypeRole,
                       GenericRole,
                       RepeatingRole };

    SocketModel& operator=(SocketModel const& other)
        {
            this->sockets = other.sockets;
            this->socketsTemplate = other.socketsTemplate;
            return *this;
        }

    SocketConstIterator begin() const;
    SocketConstIterator end() const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void addSocket(Socket&, SocketIterator);
    void removeSocket(SocketIterator);
    SocketIterator findSocket(QString const&);

    QVariantMap getSocketsTemplate();
    void setSocketsTemplate(QVariantMap const&);

    Qt::ItemFlags flags(QModelIndex const&) const;
    int rowCount(QModelIndex const& = QModelIndex()) const;
    QVariant data(QModelIndex const&, int = Qt::DisplayRole) const;
    bool setData(QModelIndex const&, QVariant const&, int = Qt::EditRole);

    QVariantMap socketsTemplate;
    SocketVector sockets{};
    std::unordered_map<QString, unsigned int> socket_counts{};

signals:
    void socketsTemplateChanged();

private slots:
    void refreshSockets();

public slots:
    void onSocketConnected(QString const&);
    void onSocketDisconnected(QString const&);
};

#endif
