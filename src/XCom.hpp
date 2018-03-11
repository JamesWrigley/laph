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

class XCom : public QObject
{
    Q_OBJECT

public:
    XCom(QObject*) = delete;
    XCom(XCom const&) = delete;
    void operator=(XCom const&) = delete;
    static XCom& get()
        {
            static XCom xcom;
            return xcom;
        }

signals:
    void wireConnected(unsigned int, QString const&);
    void wireDisconnected(unsigned int, QString const&);

private:
    XCom() { }
};

#endif
