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

#include <QVariantList>

#include "Glode.hpp"

Glode::Glode(QQuickItem* parent) : QQuickItem(parent) { }

Glode::Glode(Glode const&, QQuickItem* parent) : Glode(parent) { }

QVariant Glode::evaluate(Glode* child)
{
    std::string socket_name{this->outputs.at(child)};
    QVariant args_value{this->hooks.value(QString::fromStdString(socket_name))};
    QVariantList var_args{args_value.value<QVariantList>()};
    jl_value_t** args_array{this->args.data()};
    JL_GC_PUSHARGS(args_array, this->args.size());
    for (int i = 0; i < var_args.size(); ++i) {
        if (var_args.at(i).canConvert<double>()) {
            this->args.at(i) = jl_box_float64(var_args.at(i).value<double>());
        }
    }
    
    jl_value_t* result{jl_call(this->func, this->args.data(), this->args.size())};
    JL_GC_POP();
    if (jl_typeis(result, jl_float64_type)) {
        return QVariant(jl_unbox_float64(result));
    } else {
        return QVariant();
    }
}

QString Glode::input(QString socket_name)
{
    Glode* parent{this->inputs.at(socket_name.toStdString())};
    QVariant result{parent->evaluate(this)};

    if (result.canConvert<double>()) {
        return result.toString();
    } else {
        return "ERROR";
    }
}

void Glode::setIndex(int i)
{
    this->index = i;
    emit this->indexChanged();
}

int Glode::getIndex()
{
    return this->index;
}

void Glode::setElements(QQuickItem* elements)
{
    this->elements = elements;
    this->elements->setParentItem(this);
}

QQuickItem* Glode::getElements()
{
    return this->elements;
}

void Glode::setHooks(QVariantMap const& hooks)
{
    this->hooks = hooks;

    if (this->args.size() != (unsigned int)hooks.size()) {
        this->args.resize(hooks.size());
    }
}

QVariantMap Glode::getHooks()
{
    return this->hooks;
}
