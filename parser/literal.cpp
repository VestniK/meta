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

#include "typesystem/typesstore.h"

#include "parser/literal.h"

namespace meta {

Literal::Literal(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
    Visitable<Expression, Literal>(src, reduction)
{
    PRECONDITION(reduction.size() == 1);
    PRECONDITION(reduction[0].symbol > 0); // symbol is terminal
    PRECONDITION(reduction[0].nodes.empty());

    switch (reduction[0].symbol) {
        case meta::trueVal: mVal = trueVal; break;
        case meta::falseVal: mVal = falseVal; break;
        default: assert(false);
    }
}

} // namespace meta
