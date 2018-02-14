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
#include <QFileInfo>

#include "Glaph.hpp"

#ifdef QT_DEBUG
#include <iostream>
#endif

Glaph::Glaph(QObject* parent) : QObject(parent)
{
    jl_init();
}

Glaph::~Glaph()
{
    jl_atexit_hook(0);
}

void Glaph::addNode(QString code_path, QObject* qobj_node)
{
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

    this->nodes.insert({node->index, node});
}

void Glaph::addWire(QObject* wire_qobj)
{
    WireItem* wire{static_cast<WireItem*>(wire_qobj)};
    connect(wire, &QObject::destroyed, this, &Glaph::removeWire);
    this->wires.insert(wire);
}

QString Glaph::inputAsString(QObject* node_qobj, QString socket_name)
{
    NodeItem* node{static_cast<NodeItem*>(node_qobj)};
    std::unordered_set<WireItem*> inputs{this->getInputs(node)};
    auto wire_it{std::find_if(inputs.begin(), inputs.end(),
                              [&socket_name] (WireItem* wire) {
                                  return socket_name == wire->outputSocket;
                              })};

    if (wire_it != inputs.end()) {
        NodeItem* parent{(*wire_it)->inputNode};
        QVariant result{parent->output_values.at((*wire_it)->inputSocket)};

        if (result.isValid() && result.canConvert<double>()) {
            return result.toString();
        } else {
            return "ERROR";
        }
    } else {
        return "";
    }
}

void Glaph::removeWire(QObject* wire_qobj)
{
    this->wires.erase(static_cast<WireItem*>(wire_qobj));
}

void Glaph::evaluateFrom(NodeItem* node, QStringList outputs)
{
    std::unordered_set<WireItem*> output_wires{};
    for (auto& wire : this->getOutputs(node)) {
        if (outputs.contains(wire->inputSocket)) {
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
        QString socket_name{wire->outputSocket};
        QVariantMap other_hooks{wire->outputNode->hooks};
        emit this->inputChanged(wire->outputNode->index);

        for (auto it{other_hooks.begin()}; it != other_hooks.end(); ++it) {
            QStringList output_args{it.value().value<QStringList>()};
            if (output_args.contains(socket_name)) {
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

std::unordered_set<WireItem*> Glaph::getInputs(NodeItem* node)
{
    std::unordered_set<WireItem*> inputs{};
    for (auto& wire : this->wires) {
        if (wire->outputNode->index == node->index) {
            inputs.insert(wire);
        }
    }

    return inputs;
}

std::unordered_set<WireItem*> Glaph::getOutputs(NodeItem* node)
{
    std::unordered_set<WireItem*> outputs{};
    for (auto& wire : this->wires) {
        if (wire->inputNode == node) {
            outputs.insert(wire);
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
