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

#ifndef GLODE_HPP
#define GLODE_HPP

#include <vector>
#include <unordered_map>

#include <julia.h>
#include <QVariant>
#include <QQuickItem>
#include <QVariantMap>

class Glaph;

class Glode : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int index READ getIndex WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(QVariantMap hooks READ getHooks WRITE setHooks)
    Q_PROPERTY(QQuickItem* elements READ getElements WRITE setElements)
    Q_CLASSINFO("DefaultProperty", "elements")

public:
    Glode(QQuickItem* = Q_NULLPTR);
    Glode(Glode const&, QQuickItem* = Q_NULLPTR);

    enum Socket { Scalar, Vector, Generic };
    Q_ENUM(Socket)

    int getIndex();
    void setIndex(int);
    QQuickItem* getElements();
    void setElements(QQuickItem*);
    QVariantMap getHooks();
    void setHooks(QVariantMap const&);

    QVariant evaluate(Glode*);
    Q_INVOKABLE QString input(QString);

    QQuickItem* elements;
    unsigned int index;
    std::unordered_map<Glode*, std::string> outputs;
    std::unordered_map<std::string, Glode*> inputs;

private:
    QVariantMap hooks;
    jl_function_t* func;
    std::vector<jl_value_t*> args;

signals:
    void indexChanged();
};

#endif
