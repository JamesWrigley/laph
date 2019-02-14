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

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <vector>

#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QAbstractListModel>

#include "util.hpp"

class MessageModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum MessageLevel { Critical };
    Q_ENUM(MessageLevel)
    enum MessageRoles { MsgRole = Qt::UserRole + 1,
                        LevelRole };

    MessageModel(QObject* = Q_NULLPTR);

    void addMessage(QString const&, MessageLevel);
    void clearMessages();

    MessageModel& operator=(MessageModel const& other)
        {
            this->msgs = other.msgs;
            return *this;
        }

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    Qt::ItemFlags flags(QModelIndex const&) const;
    int rowCount(QModelIndex const& = QModelIndex()) const;
    QVariant data(QModelIndex const&, int = Qt::DisplayRole) const;

    std::vector<std::pair<QString, MessageLevel>> msgs;
};

using MessageLevel = MessageModel::MessageLevel;

#endif
