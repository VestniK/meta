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

#include "parser/binaryop.h"

namespace meta {

BinaryOp::BinaryOp(utils::array_view<StackFrame> reduction):
    Visitable<Expression, BinaryOp>(reduction)
{
    PRECONDITION(reduction.size() == 3);
    PRECONDITION(reduction[0].nodes.size() == 1);
    PRECONDITION(reduction[1].symbol > 0);
    PRECONDITION(reduction[1].nodes.empty());
    PRECONDITION(reduction[2].nodes.size() == 1);
    POSTCONDITION(mLeft != nullptr);
    POSTCONDITION(mRight != nullptr);
    mLeft = dynamic_cast<Expression*>(reduction[0].nodes.front().get());
    mRight = dynamic_cast<Expression*>(reduction[2].nodes.front().get());
    switch (reduction[1].symbol) {
        case addOp: mOp = add; break;
        case subOp: mOp = sub; break;
        case mulOp: mOp = mul; break;
        case divOp: mOp = div; break;

        case eqOp: mOp = equal; break;
        case neqOp: mOp = noteq; break;
        case lessOp: mOp = less; break;
        case lesseqOp: mOp = lesseq; break;
        case greaterOp: mOp = greater; break;
        case greatereqOp: mOp = greatereq; break;

        case andOp: mOp = boolAnd; break;
        case orOp: mOp = boolOr; break;

        default: assert(false);
    }
}

} // namespace meta
