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

#ifndef GLAPH_HPP
#define GLAPH_HPP

#include <string>
#include <unordered_map>

#include <julia.h>
#include <QObject>

#include "Glode.hpp"

class Glaph : public QObject
{
    Q_OBJECT

public:
    Glaph(QObject* = Q_NULLPTR);
    ~Glaph();

    void load_file(std::string);
    jl_value_t* safe_eval(std::string);
    Q_INVOKABLE void add_node(std::string, Glode const&);

private:
    std::unordered_map<unsigned int, Glode> nodes;
    std::unordered_map<std::string, jl_function_t*> functions;
};

#endif
