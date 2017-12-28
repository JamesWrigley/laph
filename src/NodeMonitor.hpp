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

#ifndef NODEMONITOR_HPP
#define NODEMONITOR_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>

class NodeMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString dir READ getDir WRITE setDir)
    Q_PROPERTY(QStringList nodes READ getNodes NOTIFY nodesChanged)

public:
    NodeMonitor(QObject* = Q_NULLPTR);
    QString getDir() const;
    void setDir(QString const&);
    QStringList getNodes();

private:
    QString dir;
    QStringList nodes;
    QFileSystemWatcher watcher;

    int countNodes() const;
    QString* nodeAt(int) const;

signals:
    void nodesChanged();

private slots:
    void refreshNodes();
};

#endif
