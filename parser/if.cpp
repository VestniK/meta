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

If::If(const StackFrame *reduction, size_t size): Visitable<Node, If>(reduction, size)
{
    assert(size == 6);
    const size_t thenStatementPos = 4;
    const size_t elseStatementPos = 5;
    if (!reduction[thenStatementPos].nodes.empty()) {
        assert(reduction[thenStatementPos].nodes.size() == 1);
        mThen = reduction[thenStatementPos].nodes[0];
    }
    if (!reduction[elseStatementPos].nodes.empty()) {
        assert(reduction[elseStatementPos].nodes.size() == 1);
        mElse = reduction[elseStatementPos].nodes[0];
    }
}

Node *If::condition()
{
    assert(!children.empty());
    return children[0];
}

} // namespace meta
