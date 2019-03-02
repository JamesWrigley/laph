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

#include <memory>
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include <julia.h>
#include <QObject>
#include <QUndoStack>
#include <QStringList>
#include <QQmlComponent>

#include "XCom.hpp"
#include "Socket.hpp"
#include "NodeItem.hpp"
#include "WireItem.hpp"

using NodeItemPtr = std::unique_ptr<NodeItem, std::function<void(NodeItem*)>>;

class Glaph : public QObject
{
    Q_OBJECT

public:
    Glaph(QObject* = Q_NULLPTR);
    ~Glaph();

    Q_INVOKABLE void addWire(QObject*);
    Q_INVOKABLE void removeNode(unsigned int);
    Q_INVOKABLE QObject* beginCreateNode(QString const&);
    Q_INVOKABLE void endCreateNode(QString const&, QObject*);
    Q_INVOKABLE NodeItem* getNode(unsigned int);
    Q_INVOKABLE void evaluateFrom(NodeItem*, QStringList);
    Q_INVOKABLE QString inputToString(QObject*, QString const&);
    Q_INVOKABLE QList<double> inputToList(QObject*, QString const&);
    // For some reason, QML doesn't recognize SocketType, but it does recognize
    // Socket::SocketType.
    Q_INVOKABLE Socket::SocketType getInputValueType(NodeItem*, QString const&);

    bool nodeExisted(unsigned int);
    void socketStackUndo(unsigned int);
    void socketStackRedo(unsigned int);
    WireItem const* getWire(int);
    // We need this custom replacement for QObject::findChild() because in Qt5
    // you can't use it to find an item in a Repeater. Kudos to Christian
    // Feldbacher: https://stackoverflow.com/a/22556528.
    QObject* findChildItem(QQuickItem*, QString const&);
    std::unordered_set<WireItem*> getInputs(NodeItem*, QString const& socket_name="");
    std::unordered_set<WireItem*> getOutputs(NodeItem*);

private:
    jl_value_t* safe_eval(std::string);
    template<typename T>
    T inputToType(QObject*, QString const&, std::function<T(QVariant const&)>);
    void onMainStackChange(std::function<bool(QUndoStack&)>, std::function<void(QUndoStack&)>,
                           std::function<int(QUndoStack&)>);
    void onStackChange(std::function<bool(QUndoStack&)>, std::function<void(QUndoStack&)>,
                       std::function<int(QUndoStack&)>, unsigned int, QUndoStack&);

    XCom& xcom;
    QUndoStack mainStack;
    QUndoStack socketStack;
    QQmlComponent nodeComponent;

    std::unordered_set<WireItemPtr> wires;
    std::unordered_set<unsigned int> deleted_nodes;
    std::unordered_map<unsigned int, NodeItemPtr> nodes;
    std::unordered_map<std::string, std::unordered_map<std::string, jl_function_t*>> functions;

signals:
    void inputChanged(unsigned int nodeIndex);

public slots:
    Q_INVOKABLE void removeWire(int);
};

#endif
