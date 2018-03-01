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
#include <QByteArray>
#include <QModelIndex>
#include <QVariantMap>
#include <QAbstractListModel>

#include "Socket.hpp"

class SocketModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap socketsTemplate READ getSocketsTemplate WRITE setSocketsTemplate NOTIFY socketsTemplateChanged)

public:
    SocketModel(SocketModel const&);
    SocketModel(QObject* = Q_NULLPTR,
                QVariantMap = { },
                std::vector<Socket> = { });

    enum SocketRoles { NameRole = Qt::UserRole + 1,
                       TypeRole,
                       GenericRole,
                       RepeatingRole,
                       ConnectedRole };

    SocketModel& operator=(SocketModel const& other)
        {
            this->sockets = other.sockets;
            return *this;
        }

    bool addSocket(Socket&, int = -1);
    bool removeSocket(int);

    std::vector<Socket>::const_iterator begin() const;
    std::vector<Socket>::const_iterator end() const;

    QVariantMap getSocketsTemplate();
    void setSocketsTemplate(QVariantMap const&);

    Qt::ItemFlags flags(QModelIndex const&) const;
    int rowCount(QModelIndex const& = QModelIndex()) const;
    QVariant data(QModelIndex const&, int = Qt::DisplayRole) const;
    bool setData(QModelIndex const&, QVariant const&, int = Qt::EditRole);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVariantMap socketsTemplate;
    std::vector<Socket> sockets{};

signals:
    void socketsTemplateChanged();

private slots:
    void refreshSockets();
};

#endif
