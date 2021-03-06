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

#ifndef WIREITEM_HPP
#define WIREITEM_HPP

#include <memory>

#include <QQuickItem>

class NodeItem;
class WireCommand;

class WireItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int index READ getIndex NOTIFY indexChanged)
    Q_PROPERTY(bool valid READ getValid WRITE setValid NOTIFY validChanged)
    Q_PROPERTY(QObject* endParent READ getEndParent WRITE setEndParent NOTIFY endParentChanged)
    Q_PROPERTY(QQuickItem* inputNode READ getInputNode WRITE setInputNode NOTIFY inputNodeChanged)
    Q_PROPERTY(QQuickItem* outputNode READ getOutputNode WRITE setOutputNode NOTIFY outputNodeChanged)
    Q_PROPERTY(QString inputSocket READ getInputSocket WRITE setInputSocket NOTIFY inputSocketChanged)
    Q_PROPERTY(QString outputSocket READ getOutputSocket WRITE setOutputSocket NOTIFY outputSocketChanged)
    Q_PROPERTY(QObject* initialSocket READ getInitialSocket WRITE setInitialSocket NOTIFY initialSocketChanged)

public:
    WireItem(QQuickItem* = Q_NULLPTR);
    ~WireItem();

    int getIndex();
    bool getValid();
    QObject* getEndParent();
    QString getInputSocket();
    QString getOutputSocket();
    QObject* getInitialSocket();
    QQuickItem* getInputNode();
    QQuickItem* getOutputNode();

    void setIndex(int);
    void setValid(bool);
    void setEndParent(QObject*);
    void setInputSocket(QString&);
    void setOutputSocket(QString&);
    void setInputNode(QQuickItem*);
    void setOutputNode(QQuickItem*);
    void setInitialSocket(QObject*);

    friend bool operator==(WireItem const& one, WireItem const& two)
        {
            return one.inputNode == two.inputNode && one.inputSocket == two.inputSocket;
        }

    int index;
    bool valid;
    NodeItem* inputNode;
    NodeItem* outputNode;
    QString inputSocket;
    QString outputSocket;
    WireCommand* creationCommand;

private:
    bool isNewSocket();

    QObject* endParent{nullptr};
    QObject* initialSocket{nullptr};

signals:
    void indexChanged();
    void validChanged();
    void endParentChanged();
    void inputNodeChanged();
    void outputNodeChanged();
    void inputSocketChanged();
    void outputSocketChanged();
    void initialSocketChanged();
};

using WireItemPtr = std::unique_ptr<WireItem, std::function<void(WireItem*)>>;

#endif
