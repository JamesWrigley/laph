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

#ifndef GLAPH_HPP
#define GLAPH_HPP

#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include <julia.h>
#include <QObject>
#include <QStringList>

#include "NodeItem.hpp"
#include "WireItem.hpp"

class Glaph : public QObject
{
    Q_OBJECT

public:
    Glaph(QObject* = Q_NULLPTR);
    ~Glaph();

    Q_INVOKABLE void addNode(QString, QObject*);
    Q_INVOKABLE void addWire(QObject*);
    Q_INVOKABLE QString inputAsString(QObject*, QString);
    Q_INVOKABLE void evaluateFrom(NodeItem*, QStringList);

private:
    jl_value_t* safe_eval(std::string);

    std::unordered_set<WireItem*> getInputs(NodeItem*);
    std::unordered_set<WireItem*> getOutputs(NodeItem*);

    std::unordered_set<WireItem*> wires;
    std::unordered_map<unsigned int, NodeItem*> nodes;
    std::unordered_map<std::string, std::unordered_map<std::string, jl_function_t*>> functions;

public slots:
    void removeWire(QObject*);
};

#endif
