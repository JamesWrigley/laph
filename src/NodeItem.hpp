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

#ifndef NODEITEM_HPP
#define NODEITEM_HPP

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
#include <QQmlComponent>

#include "util.hpp"
#include "XCom.hpp"
#include "Socket.hpp"
#include "WireItem.hpp"
#include "SocketModel.hpp"

using dvector = std::vector<double>;
using dvector_ptr = std::shared_ptr<dvector>;
Q_DECLARE_METATYPE(dvector_ptr);

class NodeItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int index READ getIndex WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(QObject* hooks READ getHooks WRITE setHooks NOTIFY hooksChanged)
    Q_PROPERTY(QVariantMap inputs READ getInputs WRITE setInputs NOTIFY inputsChanged)
    Q_PROPERTY(QVariantMap outputs READ getOutputs WRITE setOutputs NOTIFY outputsChanged)

public:
    NodeItem(QQuickItem* = Q_NULLPTR);
    NodeItem(NodeItem const&, QQuickItem* = Q_NULLPTR);

    // The wires need to be created from C++ instead of QML because of an
    // annoying edge-case: when the wire's original socket is
    // deleted. Presumably it's because the socket is the object parent of the
    // Wire, so some weird memory stuff happens internally in QML which causes
    // errors. Managing the wires (memory-wise) entirely from C++ avoids
    // that. The wires need to be created in two steps so that we have a chance
    // to specify the Wire property bindings before they are evaluated.
    Q_INVOKABLE QObject* beginCreateWire();
    Q_INVOKABLE void endCreateWire();

    int getIndex();
    QObject* getHooks();
    QVariantMap getInputs();
    QVariantMap getOutputs();
    void setIndex(int);
    void setHooks(QObject*);
    void setInputs(QVariantMap const&);
    void setOutputs(QVariantMap const&);

    bool isInput(QString);
    QVariantMap getHooksMap();
    Socket::SocketType getInputType(QString const&);
    Socket::SocketType getOutputType(QString const&);
    void evaluate(QString const&, std::unordered_set<WireItem*> const&);
    void cacheInput(char const*, Socket::SocketType);
    void cacheComputation(jl_value_t*, Socket::SocketType, QString const&);

    unsigned int index;
    QObject* hooks{nullptr};

    QVariantMap inputs;
    QVariantMap outputs;
    SocketModel* inputsModel{nullptr};
    SocketModel* outputsModel{nullptr};

    std::unordered_map<QString, QVariant> output_values;
    std::unordered_map<QString, dvector_ptr> vector_cache;
    std::unordered_map<std::string, jl_function_t*> functions;

private:
    XCom& xcom;
    QQmlComponent wireComponent;

    Socket::SocketType getSocketType(QString const&, SocketModel const*);

signals:
    void hooksChanged();
    void indexChanged();
    void inputsChanged();
    void outputsChanged();
    void nodeChanged(NodeItem*, QStringList);

    void wireConnectedTo(QString const&);
    void wireDisconnectedFrom(QString const&);

private slots:
    void onInputsChanged();
    void onOutputsChanged();

    void onWireConnected(unsigned int, XCom::TipType, QString const&);
    void onWireDisconnected(unsigned int, XCom::TipType, QString const&);
};

#endif
