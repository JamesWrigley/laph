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

#include "Glaph.hpp"
#include "Glode.hpp"
#include "NodeMonitor.hpp"

template<typename T>
void registerType(std::string name)
{
    qmlRegisterType<T>("Laph", 0, 1, name.c_str());
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    registerType<Glode>("Glode");
    registerType<NodeMonitor>("NodeMonitor");
    registerType<Glaph>("Glaph");

    QDir basePath{app.applicationDirPath()};
    QQmlApplicationEngine engine(basePath.filePath("src/core/main.qml"));

    return app.exec();
}
