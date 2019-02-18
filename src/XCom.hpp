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
    void requestCreateNode(QString const&, int, int, int);
    void requestDeleteNode(int);
    void requestCreateWire(unsigned int, QString const&, bool);
    void requestDeleteWire(unsigned int, QString const&, unsigned int, QString const&);
    void requestUndo();
    void requestRedo();

    // Signals to be emitted from C++land
    void createNode(QString const& nodeFile, int index, int x, int y);
    void deleteNode(int index);
    void deleteWire(int index);

private:
    XCom() { }
};

#endif
