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
#include <algorithm>
#include <cstdlib>
#include <string>

#include "utils/contract.h"
#include "utils/string.h"

#include "parser/number.h"

namespace meta {

Number::Number(utils::array_view<StackFrame> reduction):
    Visitable<Expression, Number>(reduction)
{
    PRECONDITION(reduction.size() == 1);
    PRECONDITION(std::count_if(
        reduction.begin(), reduction.end(),
        [](const StackFrame& frame) {return !frame.nodes.empty();}
    ) == 0);
    mValue = *utils::number<int>(reduction[0].tokens);
}

}
