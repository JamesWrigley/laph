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

#include <QDir>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "XCom.hpp"
#include "Glaph.hpp"
#include "Socket.hpp"
#include "NodeItem.hpp"
#include "WireItem.hpp"
#include "NodeMonitor.hpp"
#include "SocketModel.hpp"
#include "MessageModel.hpp"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

template<typename T>
void registerLaphType(char const* name)
{
    qmlRegisterType<T>("Laph", MAJOR_VERSION, MINOR_VERSION, name);
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterUncreatableType<XCom>("Laph", MAJOR_VERSION, MINOR_VERSION,
                                     "XCom", "XCom is not instantiable");
    registerLaphType<Socket>("Socket");
    registerLaphType<NodeItem>("NodeItem");
    registerLaphType<WireItem>("WireItem");
    registerLaphType<NodeMonitor>("NodeMonitor");
    registerLaphType<SocketModel>("SocketModel");
    registerLaphType<MessageModel>("MessageModel");

    QDir basePath{app.applicationDirPath()};
    QQmlApplicationEngine engine{};

    XCom& xcom{XCom::get()};
    xcom.engine = &engine;
    engine.rootContext()->setContextProperty("xcom", &xcom);

    Glaph graph{};
    engine.rootContext()->setContextProperty("graphEngine", &graph);

    // We load the QML file after setting the context properties so that the
    // file has access to the properties immediately.
    engine.load(basePath.filePath("./src/qml/core/main.qml"));

    return app.exec();
}
