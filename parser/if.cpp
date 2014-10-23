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

#include "parser/if.h"

namespace meta {

If::If(const StackFrame *reduction, size_t size): Node(reduction, size)
{
}

Node *If::condition()
{
    assert(children.size() > 1);
    return children[0];
}

Node *If::thenBlock()
{
    assert(children.size() > 1);
    return children[1]; // TODO: can we have empty then block???: "if (cond) ; else doSmth();" in this case children[1] is the else block!!!
}

Node *If::elseBlock()
{
    if (children.size() < 3)
        return nullptr;
    return children[2];
}

} // namespace meta