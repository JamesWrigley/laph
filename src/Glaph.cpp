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

Glaph::Glaph(QObject* parent) : QObject(parent)
{
    jl_init();
}

Glaph::~Glaph()
{
    jl_atexit_hook(0);
}

void Glaph::add_node(std::string code_path, Glode const& node)
{
    QString node_name{QFileInfo(QString::fromStdString(code_path)).baseName()};

    if (node.outputs.size() > 0 && functions.count(node_name.toStdString()) == 0) {
        std::smatch matches;
        std::regex func_re{"^function\\s+(\\S+)\\s+\\("};
        std::vector<std::string> func_names;

        std::string line;
        std::stringstream code;
        std::ifstream code_file{code_path};
        while (std::getline(code_file, line)) {
            if (std::regex_search(line, matches, func_re)) {
                func_names.push_back(matches[1].str());
            }

            code << line;
        }

        this->safe_eval(code.str());
        for (auto& func_name : func_names) {
            jl_function_t* func{jl_get_function(jl_base_module,
                                                func_name.c_str())};
            std::string func_label{node_name.toStdString() + "::" + func_name};
            this->functions.insert({func_label, func});
        }
    }

    nodes.insert({node.index, node});
}

void Glaph::load_file(std::string) { }

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
