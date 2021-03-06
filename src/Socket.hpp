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

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <QObject>
#include <QString>

class Socket : public QObject
{
    Q_OBJECT

public:
    enum SocketType {
        // Data types
        Scalar    = 1 << 0,
        Vector    = 1 << 1,
        Generic   = 1 << 2,
        // IO types
        Input     = 1 << 3,
        Output    = 1 << 4,
        Immediate = 1 << 5
    };
    Q_ENUM(SocketType)

    Socket(QObject* parent = Q_NULLPTR) : QObject(parent) { }
    Socket(Socket const& other, QObject* parent = Q_NULLPTR) : Socket(other.name, other.prefix,
                                                                      other.type, other.repeating,
                                                                      parent) { }
    Socket(QString const& name, QString const& prefix,
           SocketType type, bool repeating, QObject* parent = Q_NULLPTR) : QObject(parent),
                                                                           prefix(prefix),
                                                                           name(name),
                                                                           type(type),
                                                                           repeating(repeating) { }

    Socket& operator=(Socket const& other)
        {
            this->prefix = prefix;
            this->name = other.name;
            this->type = other.type;
            this->repeating = other.repeating;

            return *this;
        }

    QString prefix;
    QString name;
    SocketType type;
    bool repeating;
};

using SocketType = Socket::SocketType;

#endif
