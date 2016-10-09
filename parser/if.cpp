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

#include "parser/if.h"

namespace meta {

If::If(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
    Visitable<Node, If>(src, reduction)
{
    constexpr size_t condPos = 2;
    constexpr size_t thenPos = 4;
    constexpr size_t elsePos = 5;
    // {'if', '(', [2]<Expr>, ')', [4]<Sttmnt>, opt{'else', [5]<Sttmnt>}}
    PRECONDITION(reduction.size() == 6);
    PRECONDITION(reduction[condPos].nodes.size() == 1);
    PRECONDITION(reduction[thenPos].nodes.size() <= 1);
    PRECONDITION(reduction[elsePos].nodes.size() <= 1);
    POSTCONDITION(mConditon != nullptr);
    POSTCONDITION(mThen != nullptr || reduction[thenPos].nodes.empty());
    POSTCONDITION(mElse != nullptr || reduction[elsePos].nodes.empty());

    mConditon = dynamic_cast<Expression*>(reduction[condPos].nodes[0].get());
    if (!reduction[thenPos].nodes.empty())
        mThen = reduction[thenPos].nodes[0];
    if (!reduction[elsePos].nodes.empty())
        mElse = reduction[elsePos].nodes[0];
}

} // namespace meta
