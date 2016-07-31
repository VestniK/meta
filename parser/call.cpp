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

#include "utils/contract.h"

#include "parser/function.h"
#include "parser/call.h"
#include "parser/vardecl.h"

namespace meta {

Call::Call(utils::array_view<StackFrame> reduction):
    Visitable<Expression, Call>(reduction)
{
    // {<name>, '(', <ArgList>, ')'}
    // Все дочерние ноды это исключительно аргументы
    PRECONDITION(reduction.size() == 4);
    PRECONDITION(reduction[2].nodes.size() == countNodes(reduction));
    POSTCONDITION(mArgs.size() == reduction[2].nodes.size());
    POSTCONDITION(std::count(mArgs.begin(), mArgs.end(), nullptr) == 0);
    mFunctionName = reduction[0].tokens;
    mArgs.reserve(reduction[2].nodes.size());
    std::transform(
        reduction[2].nodes.begin(), reduction[2].nodes.end(),
        std::back_inserter(mArgs),
        [](Node* node) {return dynamic_cast<Expression*>(node);}
    );
}

void Call::setFunction(Function* func) {
    PRECONDITION(mFunction == nullptr);
    PRECONDITION(func != nullptr);
    mFunction = func;
    // use default args if needed and possible
    auto&& declaredArgs = func->args();
    for (size_t pos = mArgs.size(); pos < declaredArgs.size(); ++pos) {
        auto defaultVal = declaredArgs[pos]->initExpr();
        if (!defaultVal)
            break;
        mArgs.push_back(defaultVal);
    }
}

} // namespace meta
