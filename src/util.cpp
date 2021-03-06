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

#include <regex>
#include <iostream>

#include <QMetaObject>

#include "util.hpp"

std::ostream& operator<<(std::ostream& os, QString const& str)
{
    os << str.toStdString();
    return os;
}

/**
 * Example usage: println("Foo :0, baz :1", {"bar", "quux"})
 */
std::string fmt(std::string const& format_str, QVariantList const& args)
{
    std::string final_str{format_str};
    std::regex re{":(\\d+)"};
    auto it{std::sregex_iterator(format_str.begin(), format_str.end(), re)};
    auto end{std::sregex_iterator()};

    int length_delta{0};
    for (; it != end; ++it) {
        std::smatch match{*it};
        int index{std::stoi(match[1].str())};
        std::string replacement{args.at(index).toString().toStdString()};
        final_str.replace(match.position() + length_delta,
                          match.length(), replacement);
        length_delta += replacement.length() - match.length();
    }

    return final_str;
}

void println(std::string const& format_str, QVariantList const& args)
{
    std::cout << fmt(format_str, args) << std::endl;
}

QObject* findChildItemUnsafe(QQuickItem* parent, QString const& name)
{
    if (parent->objectName() == name) {
        return parent;
    } else {
        QList<QQuickItem*> children{parent->childItems()};
        for (auto* item : children) {
            QObject* child{findChildItemUnsafe(item, name)};
            if (child != nullptr) {
                return child;
            }
        }

        return nullptr;
    }
}

QObject* findChildItem(QQuickItem* parent, QString const& name)
{
    QObject* item{findChildItemUnsafe(parent, name)};

    if (item == nullptr) {
        throw std::runtime_error(fmt("Could not find child :0 in :1(:2)",
                                     {name, parent->metaObject()->className(), (qulonglong)parent}));
    } else {
        return item;
    }
}

QString getObjectName(QQuickItem* node, QString prefix,
                      QString socket, bool socketIsInput)
{
    QVariant name{};
    QMetaObject::invokeMethod(node, "getObjectName",
                              Q_RETURN_ARG(QVariant, name),
                              Q_ARG(QVariant, prefix),
                              Q_ARG(QVariant, socket),
                              Q_ARG(QVariant, socketIsInput));
    return name.toString();
}

bool ioTypesMatch(SocketType one, SocketType two)
{
    unsigned int mask{SocketType::Input | SocketType::Output};
    return (one & mask) == (two & mask);
}
