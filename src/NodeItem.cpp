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

#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <QMetaType>
#include <QMetaObject>
#include <QVariantList>

#include "Glaph.hpp"
#include "NodeItem.hpp"

NodeItem::NodeItem(QQuickItem* parent) : QQuickItem(parent),
                                         xcom(XCom::get()),
                                         wireComponent(xcom.engine, "./core/Wire.qml")
{
    connect(&(this->xcom), &XCom::wireDisconnected, this, &NodeItem::onWireDisconnected);
    connect(&(this->xcom), &XCom::wireConnected, this, &NodeItem::onWireConnected);
    connect(this, &NodeItem::inputsChanged, this, &NodeItem::onInputsChanged);
    connect(this, &NodeItem::outputsChanged, this, &NodeItem::onOutputsChanged);
}

NodeItem::NodeItem(NodeItem const&, QQuickItem* parent) : NodeItem(parent) { }

void NodeItem::evaluate(QString const& output_socket_name,
                        std::unordered_set<WireItem*> const& inputs)
{
    Socket const& socket{this->getSocket(output_socket_name, SocketType::Output)};

    // If this is socket is an immediate, then we can add the value directly to
    // output_values without having to call a Julia function.
    if (socket.type & SocketType::Immediate) {
        this->cacheOutput(output_socket_name, socket.type);
        return;
    }

    // If not, then we are dealing with a list of arguments to the Julia
    // function that backs this node.
    char const* socket_name_char{output_socket_name.toStdString().c_str()};
    QVariantList var_args(this->hooks->property(socket_name_char).toList());
    jl_function_t* output_function{this->functions.at(output_socket_name.toStdString())};
    std::vector<jl_value_t*> args{};

    auto box_argument{[&] (QVariant& arg, SocketType type) {
                          if (!arg.isValid()) {
                              this->output_values[output_socket_name] = QVariant();
                              return false;
                          } else if (arg.userType() == QMetaType::QString) {
                              args.push_back(jl_cstr_to_string(arg.toString().toStdString().c_str()));
                          } else if (type & SocketType::Scalar || arg.userType() == QMetaType::Double) {
                              args.push_back(jl_box_float64(arg.value<double>()));
                          } else if (type & SocketType::Vector || arg.userType() == QMetaType::type("dvector_ptr")) {
                              // For now, assume that this is a 1D vector
                              dvector_ptr vec{arg.value<dvector_ptr>()};
                              jl_value_t* vec_type{jl_apply_array_type((jl_value_t*)jl_float64_type, 1)};
                              args.push_back((jl_value_t*)jl_ptr_to_array_1d(vec_type, vec->data(), vec->size(), 0));
                          } else {
                              throw std::runtime_error("Got unknown result from node");
                          }

                          return true;
                      }};

    for (int i = 0; i < var_args.size(); ++i) {
        QVariant arg{var_args.at(i)};

        if (arg.userType() == QMetaType::Double) { // Double
            box_argument(arg, socket.type);
        } else if (arg.userType() == QMetaType::QString) {
            QString arg_str{arg.value<QString>()};

            // Check if we are dealing with an input
            if (this->isInput(arg_str)) {
                // Check if it's a repeating socket, and if so add all of its
                // repetitions to the sockets vector to process.
                std::vector<Socket const*> sockets{&(this->getSocket(arg_str, SocketType::Input))};
                if (sockets.front()->repeating) {
                    for (auto const& socket : *(this->inputsModel)) {
                        if (&socket != sockets.front() && socket.prefix == sockets.front()->prefix) {
                            sockets.push_back(&socket);
                        }
                    }
                }

                for (auto const* socket : sockets) {
                    auto wire_it{std::find_if(inputs.begin(), inputs.end(),
                                              [&] (WireItem* wire) {
                                                  return wire->outputSocket == socket->name;
                                              })};

                    // If the socket is connected
                    if (wire_it != inputs.end()) {
                        NodeItem* input_node{(*wire_it)->inputNode};
                        QVariant value{input_node->output_values.at((*wire_it)->inputSocket)};

                        // If it's a repeating socket, we also insert its name
                        if (socket->repeating) {
                            QVariant socket_name{socket->name};
                            box_argument(socket_name, socket->type);
                        }
                        if (!box_argument(value, socket->type)) {
                            return;
                        }
                    } else if (!socket->repeating) { // Otherwise, set an invalid QVariant
                        // Repeating sockets are a special case because they
                        // will always have at least one repetition unconnected,
                        // so if the socket is not connected but repeating we
                        // assume that it's that last socket. Should be possible
                        // to do it more reliably based on the position of the
                        // socket, but this will do for now.
                        this->output_values[output_socket_name] = QVariant();
                        return;
                    }
                }
            } else { // Otherwise, we just pass the string into the function
                QVariant str{arg_str};
                box_argument(str, socket.type);
            }
        }
    }

    // Prepare arguments
    jl_value_t** args_jl{};
    JL_GC_PUSHARGS(args_jl, args.size());
    for (auto i{0u}; i < args.size(); ++i) {
        args_jl[i] = args[i];
    }

    // Call function and pop arguments
    jl_value_t* result{jl_call(output_function, args_jl, args.size())};
    JL_GC_POP();

    if (jl_exception_occurred()) {
        std::cout << "Error: " << jl_typeof_str(jl_exception_occurred()) << "\n";
        this->output_values[output_socket_name] = QVariant();
    } else if ((socket.type & SocketType::Scalar && jl_typeis(result, jl_float64_type)) ||
               (socket.type & SocketType::Vector && jl_is_array(result))) {
        this->cacheComputation(result, socket.type, output_socket_name);
    } else {
        this->cacheComputation(nullptr, socket.type, output_socket_name);
    }
}

void NodeItem::cacheOutput(QString const& output_socket_name, SocketType type)
{
    // Make sure we're dealing with an input socket
    if (!(type & SocketType::Output)) {
        throw std::invalid_argument(output_socket_name.toStdString() + " is not an input");
    }

    char const* output_socket_name_char{output_socket_name.toStdString().c_str()};
    if (type & SocketType::Scalar) {
        this->output_values[output_socket_name] = this->hooks->property(output_socket_name_char);
    } else if (type & SocketType::Vector) {
        QVariantList nums(this->hooks->property(output_socket_name_char).toList());

        if (this->vector_cache.count(output_socket_name) == 0) {
            this->vector_cache.insert({output_socket_name, std::make_shared<dvector>()});
        }
        dvector_ptr& vec{this->vector_cache.at(output_socket_name)};
        vec->resize(nums.size());
        std::transform(nums.begin(), nums.end(), vec->begin(),
                       [] (QVariant const& num) {
                           return num.toDouble();
                       });

        this->output_values[output_socket_name] = QVariant::fromValue(vec);
    }
}

void NodeItem::cacheComputation(jl_value_t* result, SocketType type, QString const& key)
{
    if (result == nullptr) {
        this->output_values[key] = QVariant();
    } else if (type & SocketType::Vector) {
        if (this->vector_cache.count(key) == 0) {
            this->vector_cache.insert({key, std::make_shared<dvector>()});
        }

        double* array{(double*)jl_array_data(result)};
        dvector_ptr& vec{this->vector_cache.at(key)};
        vec->resize(jl_array_len(result));

        for (auto i{0u}; i < vec->size(); ++i) {
            vec->at(i) = array[i];
        }

        this->output_values[key] = QVariant::fromValue(vec);
    } else if (type & SocketType::Scalar) {
        this->output_values[key] = QVariant(jl_unbox_float64(result));
    }
}

void NodeItem::onWireDisconnected(unsigned int index, XCom::TipType type,
                                  QString const& socket_name)
{
    if (index == this->index) {
        // Only remove the socket if the wire is an input node with no other
        // wires connected to that socket.
        if (type == XCom::TipType::Input) {
            this->outputsModel->disconnectSocket(socket_name);
        } else {
            if (this->graphEngine->getInputs(this, socket_name).size() <= 1) {
                this->inputsModel->disconnectSocket(socket_name);
            }
        }
    }
}

void NodeItem::onWireConnected(unsigned int index, XCom::TipType type,
                               QString const& socket_name)
{
    if (index == this->index) {
        if (type == XCom::TipType::Input) {
            this->outputsModel->connectSocket(socket_name);
        } else {
            if (this->graphEngine->getInputs(this, socket_name).size() == 1) {
                this->inputsModel->connectSocket(socket_name);
            }
        }
    }
}

QObject* NodeItem::beginCreateWire()
{
    return this->wireComponent.beginCreate(this->xcom.engine->rootContext());
}

void NodeItem::endCreateWire()
{
    this->wireComponent.completeCreate();
}

bool NodeItem::isInput(QString socket_name)
{
    QVariantMap inputs(this->getInputs());
    return std::any_of(inputs.keyBegin(), inputs.keyEnd(),
                       [&] (QVariant const& socket_key) {
                           return socket_name == socket_key.toString();
                       });
}

void NodeItem::onInputsChanged()
{
    this->inputsModel = this->findChild<SocketModel*>("inputsModel");
}

void NodeItem::onOutputsChanged()
{
    this->outputsModel = this->findChild<SocketModel*>("outputsModel");
}

QVariantMap NodeItem::getHooksMap()
{
    if (this->hooks == nullptr) {
        return QVariantMap{};
    }

    QVariantMap map{};
    QMetaObject const* meta_obj{this->hooks->metaObject()};
    for (auto i{meta_obj->propertyOffset()}; i < meta_obj->propertyCount(); ++i) {
        map.insert(meta_obj->property(i).name(),
                   meta_obj->property(i).read(this->hooks));
    }

    return map;
}

Socket const& NodeItem::getSocket(QString const& socket_name,
                                  SocketType socket_type)
{
    auto* model{socket_type & SocketType::Input ? inputsModel : outputsModel};
    auto socket_it{std::find_if(model->cbegin(), model->cend(),
                                [&] (Socket const& socket) {
                                    return socket.name == socket_name;
                                })};

    if (socket_it != model->cend()) {
        return *socket_it;
    } else {
        throw std::runtime_error("Could not find socket " + socket_name.toStdString());
    }
}

int NodeItem::getIndex() { return this->index; }

void NodeItem::setIndex(int i)
{
    this->index = i;
    emit this->indexChanged();
}

QObject* NodeItem::getHooks() { return this->hooks; }

void NodeItem::setHooks(QObject* hooks)
{
    this->hooks = hooks;
    emit this->hooksChanged();
}

QVariantMap NodeItem::getOutputs() { return this->outputs; }

void NodeItem::setOutputs(QVariantMap const& outputs)
{
    this->outputs = outputs;
    emit this->outputsChanged();
}

QVariantMap NodeItem::getInputs() { return this->inputs; }

void NodeItem::setInputs(QVariantMap const& inputs)
{
    this->inputs = inputs;
    emit this->inputsChanged();
}

void NodeItem::setGraphEngine(Glaph* graphEngine)
{
    this->graphEngine = graphEngine;
}
