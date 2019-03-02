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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <ostream>

#include <QHash>
#include <QString>
#include <QVariant>
#include <QQuickItem>

#include "Socket.hpp"

std::ostream& operator<<(std::ostream&, QString const&);

namespace std {
    template<>
    struct hash<QString>
    {
        std::size_t operator()(QString const& str) const
            {
                return qHash(str);
            }
    };
}

std::string fmt(std::string const& format_str, QVariantList const& args);
void println(std::string const& format_str, QVariantList const& args={});
QObject* findChildItem(QQuickItem* parent, QString const& name);
QString getObjectName(QQuickItem*, QString, QString, bool);
bool ioTypesMatch(SocketType, SocketType);

#endif
