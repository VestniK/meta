/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "utils/contract.h"

#include "parser/assigment.h"
#include "parser/metanodes.h"
#include "parser/unexpectednode.h"

namespace meta {

Assigment::Assigment(utils::array_view<StackFrame> reduction):
    Visitable<Expression, Assigment>(reduction)
{
    PRECONDITION(reduction.size() == 3);
    PRECONDITION(reduction[0].nodes.size() == 1);
    PRECONDITION(reduction[1].nodes.empty());
    PRECONDITION(reduction[2].nodes.size() == 1);
}

Expression* Assigment::value() {
    PRECONDITION(children.size() == 2);
    return dynamic_cast<Expression*>(children.back());
}

Expression* Assigment::target() {
    PRECONDITION(children.size() == 2);
    return dynamic_cast<Expression*>(children.front());
}

}
