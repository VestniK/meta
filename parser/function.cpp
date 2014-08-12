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

#include "parser/function.h"
#include "parser/vardecl.h"

namespace meta {

Function::Function(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    static const size_t typePos = 0;
    static const size_t namePos = 1;
    static const size_t argsPos = 3;
    assert(size > argsPos + 1);
    mRetType = start[typePos].tokens;
    mName = start[namePos].tokens;
    for (auto arg : start[argsPos].nodes)
        arg->walkTopDown<meta::VarDecl>([] (meta::VarDecl *node) {node->set(meta::VarDecl::argument);}, 1);
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

} // namespace meta

