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

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <julia.h>
#include <QHash>
#include <QVariant>
#include <QQuickItem>
#include <QStringList>
#include <QVariantMap>

#include "WireItem.hpp"

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

class NodeItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int index READ getIndex WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(QObject* hooks READ getHooks WRITE setHooks NOTIFY hooksChanged)
    Q_PROPERTY(QVariantList outputs READ getOutputs WRITE setOutputs NOTIFY outputsChanged)
    Q_PROPERTY(QVariantList inputs READ getInputs WRITE setInputs NOTIFY inputsChanged)
    Q_PROPERTY(QVariantMap inputTypes READ getInputTypes NOTIFY inputTypesChanged)

public:
    NodeItem(QQuickItem* = Q_NULLPTR);
    NodeItem(NodeItem const&, QQuickItem* = Q_NULLPTR);

    enum Socket { Scalar, ScalarInput, Vector, VectorInput, Generic };
    Q_ENUM(Socket)

    int getIndex();
    QObject* getHooks();
    QVariantList getInputs();
    QVariantList getOutputs();
    void setIndex(int);
    void setHooks(QObject*);
    void setInputs(QVariantList const&);
    void setOutputs(QVariantList const&);

    bool isInput(QString);
    Socket getOutputType(QString const&);
    void evaluate(QString const&, std::unordered_set<WireItem*> const&);

    unsigned int index;
    QObject* hooks{nullptr};
    QVariantList inputs;
    QVariantList outputs;
    std::unordered_map<QString, QVariant> output_values;
    std::unordered_map<std::string, jl_function_t*> functions;

signals:
    void hooksChanged();
    void indexChanged();
    void nodeChanged(NodeItem*, QStringList);
};

#endif
