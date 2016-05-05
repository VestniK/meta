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

#include "parser/prefixop.h"

namespace meta {

PrefixOp::PrefixOp(utils::array_view<StackFrame> reduction):
    Visitable<Expression, PrefixOp>(reduction)
{
    PRECONDITION(reduction.size() == 2);
    PRECONDITION(reduction[0].symbol > 0); // symbol is terminal
    PRECONDITION(reduction[1].nodes.size() == 1);
    switch (reduction[0].symbol) {
        case subOp: mOperation = negative; break;
        case addOp: mOperation = positive; break;
        case notOp: mOperation = boolnot; break;
        default: assert(false);
    }
}

Node *PrefixOp::operand()
{
    PRECONDITION(children.size() == 1);
    return children[0];
}

} // namespace meta
