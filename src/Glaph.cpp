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

#include <regex>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <QString>
#include <QtGlobal>
#include <QFileInfo>
#include <QQmlEngine>

#include "Glaph.hpp"
#include "UndoCommands.hpp"

JULIA_DEFINE_FAST_TLS();

Glaph::Glaph(QObject* parent) : QObject(parent),
                                xcom(XCom::get()),
                                mainStack(parent),
                                socketStack(parent),
                                nodeComponent(xcom.engine),
                                importer(&this->sourceTree, &this->errorCollector)
{
    // Initialize the Julia runtime
    jl_init();

    // Connect signals
    connect(&xcom, &XCom::requestCreateNode, [&] (QString const& nodeFile, int index, int x, int y) {
                                                 this->mainStack.push(new CreateNode(*this, nodeFile, index, x, y));
                                             });
    connect(&xcom, &XCom::requestDeleteNode, [&] (int index) {
                                                 this->mainStack.push(new DeleteNode(*this, this->nodes.at(index).get()));
                                             });
    connect(&xcom, &XCom::requestCreateWire, [&] (int startIndex, QString const& startSocket, bool isInput) {
                                                 this->mainStack.push(new CreateWire(*this, startIndex, startSocket, isInput));
                                             });
    connect(&xcom, &XCom::requestDeleteWire, [&] (int wireIndex) {
                                                 this->mainStack.push(new DeleteWire(*this, wireIndex));
                                             });
    connect(&xcom, &XCom::requestReconnectWireTip, [&] (unsigned int wireIndex, XCom::TipType tipType, unsigned int newNodeIndex, QString const& newSocket) {
                                                       this->mainStack.push(new ReconnectWireTip(*this, wireIndex, tipType, newNodeIndex, newSocket));
                                                   });
    connect(&xcom, &XCom::requestCreateSocket, [&] (Socket const& socket, unsigned int nodeIndex, unsigned int socketIndex) {
                                                   this->socketStack.push(new CreateSocket(socket, nodeIndex, socketIndex));
                                               });
    connect(&xcom, &XCom::requestDeleteSocket, [&] (Socket const& socket, unsigned int nodeIndex, unsigned int socketIndex) {
                                                   this->socketStack.push(new DeleteSocket(socket, nodeIndex, socketIndex));
                                               });
    connect(&xcom, &XCom::nodeMoved, [&] (unsigned int nodeIndex, int oldX, int oldY, int newX, int newY) {
                                         this->mainStack.push(new MoveNode(*this, nodeIndex, oldX, oldY, newX, newY));
                                     });

    connect(&xcom, &XCom::requestUndo, [&] () {
                                           this->onMainStackChange(std::mem_fn(&QUndoStack::canUndo), std::mem_fn(&QUndoStack::undo),
                                                                   [] (auto& s) { return s.index() - 1; });
                                       });
    connect(&xcom, &XCom::requestRedo, [&] () {
                                           this->onMainStackChange(std::mem_fn(&QUndoStack::canRedo), std::mem_fn(&QUndoStack::redo),
                                                                   std::mem_fn(&QUndoStack::index));
                                       });

    // Set up protobuf
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    this->sourceTree.MapPath("", "proto");
    pb::FileDescriptor const* fileDescriptor{this->importer.Import("nodes.proto")};

    // Check that all the message fields are supported
    for (int i{0}; i < fileDescriptor->message_type_count(); ++i) {
        pb::Descriptor const* messageDescriptor{fileDescriptor->message_type(i)};
        for (int j{0}; j < messageDescriptor->field_count(); ++j) {
            pb::FieldDescriptor const* fieldDescriptor{messageDescriptor->field(j)};
            bool supported{std::any_of(this->supportedTypes.begin(), this->supportedTypes.end(),
                                       [&] (auto& t) { return t == fieldDescriptor->type(); })};

            if (!supported) {
                throw std::runtime_error(fmt("Field type ':0' of :1 is not supported",
                                             {fieldDescriptor->type(), QString::fromStdString(fieldDescriptor->full_name())}));
            }
        }
    }
}

Glaph::~Glaph()
{
    jl_atexit_hook(0);
    pb::ShutdownProtobufLibrary();
}

void Glaph::onMainStackChange(std::function<bool(QUndoStack&)> predicate, std::function<void(QUndoStack&)> action,
                              std::function<int(QUndoStack&)> index)
{
    if (predicate(this->mainStack)) {
        unsigned int eventId{static_cast<BaseCommand const*>(this->mainStack.command(index(this->mainStack)))->eventId};
        this->onStackChange(predicate, action, index, eventId, this->mainStack);
    }
}

void Glaph::socketStackUndo(unsigned int eventId)
{
    this->onStackChange(std::mem_fn(&QUndoStack::canUndo), std::mem_fn(&QUndoStack::undo),
                        [] (auto& s) { return s.index() - 1; }, eventId, this->socketStack);
}

void Glaph::socketStackRedo(unsigned int eventId)
{
    this->onStackChange(std::mem_fn(&QUndoStack::canRedo), std::mem_fn(&QUndoStack::redo),
                        std::mem_fn(&QUndoStack::index), eventId, this->socketStack);
}

void Glaph::onStackChange(std::function<bool(QUndoStack&)> predicate, std::function<void(QUndoStack&)> action,
                          std::function<int(QUndoStack&)> index, unsigned int eventId, QUndoStack& stack)
{
    auto nextCommand{[&] () { return static_cast<BaseCommand const*>(stack.command(index(stack))); }};

    while (predicate(stack) && nextCommand()->eventId == eventId) {
        action(stack);
    }
}

QObject* Glaph::beginCreateNode(QString const& node_path)
{
    QFileInfo fileInfo{node_path};

    this->nodeComponent.loadUrl(node_path);
    NodeItem* node_ptr{static_cast<NodeItem*>(this->nodeComponent.beginCreate(this->xcom.engine->rootContext()))};
    node_ptr->nodeFile = fileInfo.fileName();
    node_ptr->setGraphEngine(this);

    pb::Descriptor const* descriptor{this->importer.pool()->FindMessageTypeByName(fileInfo.baseName().toStdString())};
    if (descriptor != NULL) {
        node_ptr->setMessagePrototype(this->messageFactory.GetPrototype(descriptor));
    }

    QQmlEngine::setObjectOwnership(node_ptr, QQmlEngine::CppOwnership);
    return node_ptr;
}

void Glaph::endCreateNode(QString const& code_path, QObject* qobj_node)
{
    this->nodeComponent.completeCreate();
    NodeItem* node{static_cast<NodeItem*>(qobj_node)};
    connect(node, &NodeItem::nodeChanged, this, &Glaph::evaluateFrom);
    QString node_name{QFileInfo(code_path).baseName()};

    if (node->outputs.size() > 0) {
        if (this->functions.count(node_name.toStdString()) == 0) {
            std::smatch matches{};
            std::regex func_re{"^function\\s+(\\S+)\\s*\\("};
            std::vector<std::string> func_names{};
            std::unordered_map<std::string, jl_function_t*> funcs_map{};

            std::string line{};
            std::stringstream code{};
            std::ifstream code_file{code_path.toStdString()};
            while (std::getline(code_file, line)) {
                if (std::regex_search(line, matches, func_re)) {
                    func_names.push_back(matches[1].str());
                }

                code << line << "\n";
            }

            this->safe_eval(code.str());
            for (auto& func_name : func_names) {
                jl_function_t* func{jl_get_function(jl_main_module,
                                                    func_name.c_str())};
                funcs_map.insert({func_name, func});
            }

            this->functions.insert({node_name.toStdString(), funcs_map});
        }

        node->functions = this->functions.at(node_name.toStdString());
    }

    this->nodes.insert({node->index, NodeItemPtr(node,
                                                 [] (NodeItem* node) {
                                                     node->deleteLater();
                                                 })});
}

NodeItem* Glaph::getNode(unsigned int index)
{
    auto node_it{this->nodes.find(index)};
    if (node_it == this->nodes.end()) {
        throw std::runtime_error(fmt("Could not find node with index :0", {index}));
    } else {
        return node_it->second.get();
    }
}

void Glaph::addWire(QObject* wire_qobj)
{
    WireItem* wire{static_cast<WireItem*>(wire_qobj)};
    QQmlEngine::setObjectOwnership(wire, QQmlEngine::CppOwnership);
    this->wires.emplace(wire, [] (WireItem* wire) { wire->deleteLater(); });
}

QString Glaph::inputToString(QObject* node_qobj, QString const& socket_name)
{
    return this->inputToType<QString>(node_qobj, socket_name,
                             [] (QVariant const& var) {
                                 if (var.canConvert<double>()) {
                                     return var.toString();
                                 } else {
                                     return QString("ERROR");
                                 }
                             });
}

QList<qreal> Glaph::inputToList(QObject* node_qobj, QString const& socket_name)
{
    return this->inputToType<QList<qreal>>(node_qobj, socket_name,
                                           [] (QVariant const& var) {
                                               QList<qreal> list{};

                                               if (var.canConvert<dvector_ptr>()) {
                                                   dvector_ptr vec{var.value<dvector_ptr>()};
                                                   for (auto& num : *vec) {
                                                       list.append(num);
                                                   }
                                               }

                                               return list;
                                           });
}

template<typename T>
T Glaph::inputToType(QObject* node_qobj, QString const& socket_name,
                     std::function<T(QVariant const&)> func)
{
    NodeItem* node{static_cast<NodeItem*>(node_qobj)};
    std::unordered_set<WireItem*> inputs{this->getInputs(node)};
    auto wire_it{std::find_if(inputs.begin(), inputs.end(),
                              [&socket_name] (WireItem* wire) {
                                  return socket_name == wire->outputSocket;
                              })};

    if (wire_it != inputs.end()) {
        NodeItem* input{(*wire_it)->inputNode};
        QVariant result{input->output_values[(*wire_it)->inputSocket]};

        return func(result);
    } else {
        throw std::runtime_error("Could not find socket: " + socket_name.toStdString());
    }
}

void Glaph::removeWire(int index)
{
    auto wire_it{std::find_if(this->wires.begin(), this->wires.end(),
                              [&index] (WireItemPtr const& wire_ptr) {
                                  return wire_ptr->index == index;
                              })};

    this->wires.erase(wire_it);
}

void Glaph::removeNode(unsigned int index)
{
    NodeItem* node{this->nodes.at(index).get()};
    auto wire_cleanup{[&] (auto&& wires) {
                          for (auto& wire : wires) {
                              emit this->xcom.requestDeleteWire(wire->index);
                              emit this->xcom.wireDisconnected(wire->outputNode->index, XCom::TipType::Output, wire->outputSocket);
                              emit this->xcom.wireDisconnected(wire->inputNode->index, XCom::TipType::Input, wire->inputSocket);
                          }
                      }};
    auto socket_cleanup{[&] (SocketModel model) {
                            for (auto socket_it{model.cbegin()}; socket_it != model.cend(); ++socket_it) {
                                emit this->xcom.requestDeleteSocket(*socket_it, index, socket_it - model.cbegin());
                            }
                        }};

    wire_cleanup(this->getInputs(node));
    wire_cleanup(this->getOutputs(node));
    socket_cleanup(node->inputsModel);
    socket_cleanup(node->outputsModel);

    this->deleted_nodes.insert(index);
    this->nodes.erase(index);
}

bool Glaph::nodeExisted(unsigned int nodeIndex)
{
    return this->deleted_nodes.find(nodeIndex) != this->deleted_nodes.end();
}

void Glaph::evaluateFrom(NodeItem* node, QStringList outputs)
{
    // Find all wires that are connected to the affected output sockets
    std::unordered_set<WireItem*> output_wires{};
    for (auto& wire : this->getOutputs(node)) {
        if (wire->valid && outputs.contains(wire->inputSocket)) {
            output_wires.insert(wire);
        }
    }

    // Evaluate all affected output values (looking backwards)
    for (auto& wire : output_wires) {
        node->evaluate(wire->inputSocket, this->getInputs(node));
    }

    // Find all nodes that have this node as an input
    std::unordered_map<NodeItem*, QStringList> dirtied_outputs{};
    for (auto& wire : output_wires) {
        Socket const& socket{wire->outputNode->getSocket(wire->outputSocket, SocketType::Input)};
        QVariantMap other_hooks{wire->outputNode->getHooksMap()};
        emit this->inputChanged(wire->outputNode->index);

        for (auto it{other_hooks.begin()}; it != other_hooks.end(); ++it) {
            QStringList output_args{it.value().value<QStringList>()};
            if (output_args.contains(socket.name) ||
                (socket.repeating && output_args.contains(socket.prefix))) {
                if (dirtied_outputs.count(wire->outputNode) > 0) {
                    dirtied_outputs.at(wire->outputNode).append(it.key());
                } else {
                    dirtied_outputs.insert({wire->outputNode, QStringList{it.key()}});
                }
            }
        }
    }

    // And then evaluate them
    for (auto it{dirtied_outputs.begin()}; it != dirtied_outputs.end(); ++it) {
        this->evaluateFrom(it->first, it->second);
    }
}

Socket::SocketType Glaph::getInputValueType(NodeItem* node, QString const& socket)
{
    auto inputs{this->getInputs(node)};
    auto wire_it{std::find_if(inputs.begin(), inputs.end(),
                              [&socket] (WireItem* wire) {
                                  return wire->outputSocket == socket;
                              })};

    if (wire_it != inputs.end()) {
        WireItem* wire{*wire_it};
        bool isVector{wire->inputNode->output_values.at(wire->inputSocket).canConvert<dvector_ptr>()};
        return isVector ? SocketType::Vector : SocketType::Scalar;
    } else {
        throw std::runtime_error("Could not find wire with outputSocket: '"
                                 + socket.toStdString() + "'");
    }
}

WireItem const* Glaph::getWire(int wireIndex)
{
    auto wire_it{std::find_if(this->wires.cbegin(), this->wires.cend(),
                              [&] (auto const& wire) {
                                  return wireIndex == wire->index;
                              })};

    if (wire_it == this->wires.cend()) {
        throw std::runtime_error(fmt("Could not find wire with index :0", {wireIndex}));
    } else {
        return wire_it->get();
    }
}

std::unordered_set<WireItem*> Glaph::getInputs(NodeItem* node, QString const& socket_name)
{
    std::unordered_set<WireItem*> inputs{};
    for (auto& wire : this->wires) {
        if (wire->valid && wire->outputNode->index == node->index &&
            (socket_name != "" ? wire->outputSocket == socket_name : true)) {
            inputs.insert(wire.get());
        }
    }

    return inputs;
}

std::unordered_set<WireItem*> Glaph::getOutputs(NodeItem* node)
{
    std::unordered_set<WireItem*> outputs{};
    for (auto& wire : this->wires) {
        if (wire->valid && wire->inputNode == node) {
            outputs.insert(wire.get());
        }
    }

    return outputs;
}

jl_value_t* Glaph::safe_eval(std::string code)
{
    if (!jl_is_initialized()) {
        throw std::runtime_error("Julia not initialized");
    }

    jl_value_t* result{jl_eval_string(code.c_str())};
    if (jl_exception_occurred()) {
        std::stringstream error{"Error evaluating: '"};
        error << code.substr(0, 50) << "'\n"
              << jl_typeof_str(jl_exception_occurred()) << "\n";
        throw std::runtime_error(error.str());
    } else {
        return result;
    }
}
