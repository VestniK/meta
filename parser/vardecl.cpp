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

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(utils::array_view<StackFrame> reduction):
    Visitable<Declaration, VarDecl>(reduction)
{
    // {<type>, <name>, opt{'=', 'Expr'}}
    PRECONDITION(reduction.size() == 3);
    PRECONDITION(countNodes(reduction) == reduction[2].nodes.size());
    PRECONDITION(reduction[2].nodes.size() <= 1);
    POSTCONDITION(reduction[2].nodes.empty() || mInitExpr != nullptr);
    mTypeName = reduction[0].tokens;
    mName = reduction[1].tokens;
    if (!reduction[2].nodes.empty())
        mInitExpr = dynamic_cast<Expression*>(reduction[2].nodes[0].get());
}

const Declaration::AttributesMap& VarDecl::attributes() const {
    static AttributesMap res;
    return res;
}

}
