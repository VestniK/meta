/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include "parser/struct.h"

namespace meta {

Struct::Struct(utils::array_view<StackFrame> reduction):
    Visitable<Declaration, Struct>(reduction)
{
    auto it = std::find_if(reduction.begin(), reduction.end(), [](const StackFrame& frame) {
        return frame.symbol == Terminal::structKeyword;
    });
    assert(it != reduction.end());
    ++it;
    assert(it != reduction.end());
    assert(it->symbol == Terminal::identifier);
    mName = it->tokens;
}

const Declaration::AttributesMap Struct::attrMap = {};

}
