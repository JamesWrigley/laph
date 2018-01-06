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
#include <utility>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <QString>
#include <QFileInfo>

#include "Glaph.hpp"

Glaph::Glaph()
{
    jl_init();
}

Glaph::~Glaph()
{
    jl_atexit_hook(0);
}

void Glaph::add_node(std::string code_path, unsigned int index)
{
    QString node_name{QFileInfo(QString::fromStdString(code_path)).baseName()};
    jl_function_t* node_func;

    if (functions.count(node_name.toStdString()) == 0) {
        std::smatch matches;
        std::regex func_re{"^function\\s+(\\S+)\\("};
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
        
        jl_eval_string(code.str().c_str());
        node_func = jl_get_function(jl_base_module, func_names.at(0).c_str());
    } else {
        node_func = functions.at(node_name.toStdString());
    }

    Glode node{index, node_func};
    nodes.insert({index, node});
}

void Glaph::load_file(std::string) { }

jl_value_t* safe_eval(std::string code)
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
