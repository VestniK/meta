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

#include <cassert>

#include "parser/prefixop.h"

namespace meta {

PrefixOp::PrefixOp(AST *ast, const StackFrame *start, size_t size): Node(ast, start, size)
{
    assert(size == 2);
    switch (*start[0].tokens.begin()->start) {
        case '-': mOperation = negative; break;
        case '+': mOperation = positive; break;
    }
}

} // namespace meta
