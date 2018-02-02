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

#include <regex>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <QString>
#include <QFileInfo>

#include "Glaph.hpp"

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
    connect(node, &NodeItem::inputChanged, this, &Glaph::evaluateFrom);
    QString node_name{QFileInfo(code_path).baseName()};

    if (node->outputs_map.size() > 0) {
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
    // std::cout << "inputAsString()\n";
    NodeItem* node{static_cast<NodeItem*>(node_qobj)};
    InputMap inputs{this->getInputsMap(node)};

    if (inputs.count(socket_name) > 0) {
        NodeItem* parent{inputs.at(socket_name)};
        QVariant result{parent->evaluate(socket_name, inputs)};

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
    // std::cout << "evaluateFrom(" << node->index << ")\n";
    std::unordered_set<WireItem*> output_wires{this->getOutputs(node)};
    InputMap inputs{this->getInputsMap(node)};
    for (auto& wire : output_wires) {
        if (outputs.contains(wire->outputSocket)) {
            node->evaluate(wire->outputSocket, this->getInputsMap(node));
        }
    }
}

InputMap Glaph::getInputsMap(NodeItem* node)
{
    InputMap inputs{};
    for (auto& wire : this->getInputs(node)) {
        inputs.insert({wire->inputSocket, wire->inputNode});
    }

    return inputs;
}

std::unordered_set<WireItem*> Glaph::getInputs(NodeItem* node)
{
    std::unordered_set<WireItem*> inputs{};
    for (auto& wire : this->wires) {
        if (wire->outputNode == node) {
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
