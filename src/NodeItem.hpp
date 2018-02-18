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

#include <memory>
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

using dvector = std::vector<double>;
using dvector_ptr = std::shared_ptr<dvector>;
Q_DECLARE_METATYPE(dvector_ptr);

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
    Q_PROPERTY(QList<bool> inputTypeSwaps READ getInputTypeSwaps WRITE setInputTypeSwaps NOTIFY inputTypeSwapsChanged)
    Q_PROPERTY(QList<bool> outputTypeSwaps READ getOutputTypeSwaps WRITE setOutputTypeSwaps NOTIFY outputTypeSwapsChanged)

public:
    NodeItem(QQuickItem* = Q_NULLPTR);
    NodeItem(NodeItem const&, QQuickItem* = Q_NULLPTR);

    enum Socket { Scalar, ScalarInput, Vector, VectorInput, Generic };
    Q_ENUM(Socket)

    int getIndex();
    QObject* getHooks();
    QVariantList getInputs();
    QVariantList getOutputs();
    QList<bool> getInputTypeSwaps();
    QList<bool> getOutputTypeSwaps();
    void setIndex(int);
    void setHooks(QObject*);
    void setInputs(QVariantList const&);
    void setOutputs(QVariantList const&);
    void setInputTypeSwaps(QList<bool> const&);
    void setOutputTypeSwaps(QList<bool> const&);

    bool isInput(QString);
    QVariantMap getHooksMap();
    Socket getInputType(QString const&);
    Socket getOutputType(QString const&);
    void evaluate(QString const&, std::unordered_set<WireItem*> const&);
    void cache(char const*, Socket);

    unsigned int index;
    QObject* hooks{nullptr};
    QVariantList inputs;
    QVariantList outputs;
    QList<bool> inputTypeSwaps;
    QList<bool> outputTypeSwaps;
    std::unordered_map<QString, QVariant> output_values;
    std::unordered_map<QString, dvector_ptr> vector_cache;
    std::unordered_map<std::string, jl_function_t*> functions;

private:
    Socket getSocketType(QString const&, QVariantList const&, QList<bool> const&);

signals:
    void hooksChanged();
    void indexChanged();
    void inputsChanged();
    void outputsChanged();
    void inputTypeSwapsChanged();
    void outputTypeSwapsChanged();
    void nodeChanged(NodeItem*, QStringList);
};

#endif
