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

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(const StackFrame* start, size_t size): VisitableNode<VarDecl>(start, size), mFlags(0)
{
    assert(size == 3);
    mTypeName = start[0].tokens;
    mName = start[1].tokens;
}

bool VarDecl::is(VarDecl::Flags flag) const
{
    return (mFlags & flag) != 0;
}

void VarDecl::set(VarDecl::Flags flag, bool val)
{
    mFlags = val ? (mFlags | flag) : (mFlags & ~flag);
}

bool VarDecl::inited() const
{
    return !children.empty();
}

Node *VarDecl::initExpr()
{
    return children.empty() ? nullptr : children.front();
}

}
