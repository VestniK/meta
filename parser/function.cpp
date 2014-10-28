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

Function::Function(const StackFrame *reduction, size_t size): Node(reduction, size)
{
    static const size_t visibilityPos = 0;
    static const size_t typePos = 1;
    static const size_t namePos = 2;
    static const size_t argsPos = 4;
    assert(size > argsPos + 1);
    auto visTokenIt = reduction[visibilityPos].tokens.begin();
    if (visTokenIt != reduction[visibilityPos].tokens.end())
        mVisibility = fromToken(*visTokenIt);
    mRetType = reduction[typePos].tokens;
    mName = reduction[namePos].tokens;
    for (auto arg : reduction[argsPos].nodes)
        walkTopDown<meta::VarDecl>(*arg, [] (meta::VarDecl *node) {node->set(meta::VarDecl::argument); return false;}, 0);
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

} // namespace meta

