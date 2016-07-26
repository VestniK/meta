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

#include "parser/function.h"
#include "parser/call.h"
#include "parser/vardecl.h"

namespace meta {

Call::Call(utils::array_view<StackFrame> reduction):
    Visitable<Expression, Call>(reduction),
    mChildren(getNodes(reduction)),
    mFunction(nullptr)
{
    PRECONDITION(reduction.size() == 4);
    mFunctionName = reduction[0].tokens;
}

void Call::setFunction(Function* func) {
    mFunction = func;
    // use default args if needed and possible
    auto declaredArgs = func->args();
    for (size_t pos = mChildren.size(); pos < declaredArgs.size(); ++pos) {
        auto defaultVal = declaredArgs[pos]->initExpr();
        if (!defaultVal)
            break;
        mChildren.push_back(defaultVal);
    }
}

} // namespace meta
