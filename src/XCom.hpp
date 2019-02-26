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

#ifndef XCOM_HPP
#define XCOM_HPP

#include <QObject>
#include <QQmlApplicationEngine>

#include "Socket.hpp"
#include "WireItem.hpp"

class XCom : public QObject
{
    Q_OBJECT

public:
    // It'd be nicer if this was defined in WireItem.hpp, but enums that are
    // used as signal parameters from QML need to be declared in the same class
    // as the signal (i.e. wireConnected() and wireDisconnected()).
    enum class TipType { Input, Output };
    Q_ENUM(TipType)

    enum class ConnectionType { New, Reconnect, None };
    Q_ENUM(ConnectionType)

    XCom(QObject*) = delete;
    XCom(XCom const&) = delete;
    void operator=(XCom const&) = delete;
    static XCom& get()
        {
            static XCom xcom;
            return xcom;
        }

    QQmlApplicationEngine* engine;

signals:
    void repaintCanvas();

    void wireConnected(unsigned int, TipType, QString const&);
    void wireDisconnected(unsigned int, TipType, QString const&);

    // Signals to be emitted from QMLland
    void requestCreateNode(QString const& nodeFile, int nodeIndex, int x, int y);
    void requestDeleteNode(int nodeIndex);
    void requestCreateWire(unsigned int startNode, QString const& startSocket, bool isInput);
    void requestDeleteWire(int wireIndex);
    void requestReconnectWireTip(unsigned int wireIndex, TipType tipType,
                                 unsigned int nodeIndex, QString const& newSocket);
    void requestUndo();
    void requestRedo();

    // Signals to be emitted from C++land
    void requestCreateSocket(Socket const&, unsigned int nodeIndex, unsigned int socketIndex);
    void requestDeleteSocket(Socket const&, unsigned int nodeIndex, unsigned int socketIndex);
    void createSocket(Socket, unsigned int nodeIndex, unsigned int socketIndex);
    void deleteSocket(SocketType, unsigned int nodeIndex, unsigned int socketIndex);
    void createNode(QString const& nodeFile, int index, int x, int y);
    void deleteNode(int index);
    void deleteWire(int index);
    void connectWireTip(QObject* wireTip, QObject* target, ConnectionType type);
    void reconnectWireTip(QObject* wireTip, QObject* target, ConnectionType type, bool isReplay);

private:
    XCom() { }
};

using TipType = XCom::TipType;
using ConnectionType = XCom::ConnectionType;

#endif
