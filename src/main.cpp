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
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "NodeMonitor.hpp"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QDir basePath{app.applicationDirPath()};
    qmlRegisterType<NodeMonitor>("laph", 0, 1, "NodeMonitor");
    QQmlApplicationEngine engine(basePath.filePath("src/core/main.qml"));

    return app.exec();
}
