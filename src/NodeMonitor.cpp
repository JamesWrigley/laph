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
#include <QCoreApplication>

#include "NodeMonitor.hpp"

NodeMonitor::NodeMonitor(QObject* parent) : QObject(parent)
{
    connect(&(this->watcher), &QFileSystemWatcher::directoryChanged,
            [&] (const QString&) { this->refreshNodes(); });
}

QString NodeMonitor::getDir() const
{
    return this->dir;
}

void NodeMonitor::setDir(const QString& new_dir)
{
    QDir basePath{QCoreApplication::instance()->applicationDirPath()};
    this->dir = basePath.filePath(new_dir);

    if (!this->watcher.directories().empty()) {
        this->watcher.removePaths(this->watcher.directories());
    }

    this->watcher.addPath(this->dir);
    this->refreshNodes();
    emit this->nodesChanged();
}

void NodeMonitor::refreshNodes()
{
    QDir qdir(this->dir, "*.qml", QDir::Name, QDir::Files);
    this->nodes = qdir.entryList();
    emit this->nodesChanged();
}

QStringList NodeMonitor::getNodes()
{
    return nodes;
}
