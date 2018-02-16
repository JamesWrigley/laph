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

#include "NodeItem.hpp"

NodeItem::NodeItem(QQuickItem* parent) : QQuickItem(parent) { }

NodeItem::NodeItem(NodeItem const&, QQuickItem* parent) : NodeItem(parent) { }

void NodeItem::evaluate(QString const& output_socket_name,
                        std::unordered_set<WireItem*> const& inputs)
{
    Socket type{this->getOutputType(output_socket_name)};
    char const* socket_name_char{output_socket_name.toStdString().c_str()};

    // If this is a data input node, then we can add the values directly to
    // output_values without having to call a Julia function.
    if (type == ScalarInput) {
        this->output_values[output_socket_name] = this->hooks->property(socket_name_char);
        return;
    } else if (type == VectorInput) {
        // For some reason when dealing with a JS array we can only access it
        // through the hooks metaobject.
        QVariant var{this->getHooksMap().value(output_socket_name)};
        this->output_values[output_socket_name] = var;

        return;
    }

    // We assume that if this is not an input node, then we are dealing with a
    // list of arguments to the Julia function.
    QVariantList var_args(this->hooks->property(socket_name_char).toList());
    jl_function_t* output_function{this->functions.at(output_socket_name.toStdString())};
    jl_value_t** args{};
    JL_GC_PUSHARGS(args, var_args.size());

    for (int i = 0; i < var_args.size(); ++i) {
        QVariant arg{var_args.at(i)};

        if (arg.userType() == QMetaType::Double) { // Double
            args[i] = jl_box_float64(arg.value<double>());
        } else if (arg.userType() == QMetaType::QString) {
            QString arg_str{arg.value<QString>()};

            // Check if we are dealing with an input
            if (this->isInput(arg_str)) {
                auto wire_it{std::find_if(inputs.begin(), inputs.end(),
                                          [&arg_str] (WireItem* wire) {
                                              return wire->outputSocket == arg_str;
                                          })};

                // If the socket is connected
                if (wire_it != inputs.end()) {
                    NodeItem* input_node{(*wire_it)->inputNode};
                    QVariant value{input_node->output_values.at((*wire_it)->inputSocket)};

                    if (!value.isValid()) { // If the node can't compute its result
                        JL_GC_POP(); // Pop arguments
                        this->output_values[output_socket_name] = QVariant();
                        return;
                    } else if (value.canConvert<double>()) {
                        args[i] = jl_box_float64(value.value<double>());
                    } else {
                        throw std::runtime_error("Got non-double result from node");
                    }
                } else { // Otherwise, set an invalid QVariant
                    JL_GC_POP(); // Pop arguments
                    this->output_values[output_socket_name] = QVariant();
                    return;
                }
            } else {
                args[i] = jl_cstr_to_string(arg_str.toStdString().c_str());
            }
        }
    }

    jl_value_t* result{jl_call(output_function, args, var_args.size())};
    if (jl_exception_occurred()) {
        std::cout << "Error: " << jl_typeof_str(jl_exception_occurred()) << "\n";
        this->output_values[output_socket_name] = QVariant();
    } else if (type == Scalar && jl_typeis(result, jl_float64_type)) {
        this->output_values[output_socket_name] = QVariant(jl_unbox_float64(result));
    }

    JL_GC_POP(); // Pop arguments
}

bool NodeItem::isInput(QString socket_name)
{
    QVariantList inputs(this->getInputs());
    return std::any_of(inputs.begin(), inputs.end(), [&] (QVariant socket) {
            return socket_name == socket.toList().first().toString();
        });
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

NodeItem::Socket NodeItem::getInputType(QString const& socket)
{
    return this->getSocketType(socket, this->inputs);
}

NodeItem::Socket NodeItem::getOutputType(QString const& socket)
{
    return this->getSocketType(socket, this->outputs);
}

NodeItem::Socket NodeItem::getSocketType(QString const& socket, QVariantList const& sockets)
{
    auto output_it{std::find_if(sockets.begin(), sockets.end(),
                                [&] (QVariant const& output) {
                                    return output.toList().at(0).toString() == socket;
                                })};
    if (output_it != this->outputs.end()) {
        return output_it->toList().at(1).value<NodeItem::Socket>();
    } else {
        throw std::runtime_error("Could not find socket " + socket.toStdString());
    }
}

void NodeItem::setIndex(int i)
{
    this->index = i;
    emit this->indexChanged();
}

int NodeItem::getIndex()
{
    return this->index;
}

void NodeItem::setHooks(QObject* hooks)
{
    this->hooks = hooks;
    emit this->hooksChanged();
}

QObject* NodeItem::getHooks()
{
    return this->hooks;
}

void NodeItem::setOutputs(QVariantList const& outputs)
{
    this->outputs = outputs;
}

QVariantList NodeItem::getOutputs()
{
    return this->outputs;
}

void NodeItem::setInputs(QVariantList const& inputs)
{
    this->inputs = inputs;
}

QVariantList NodeItem::getInputs()
{
    return this->inputs;
}
