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
#include <cassert>
#include <functional>
#include <iterator>
#include <type_traits>

#include "utils/contract.h"

#include "parser/metanodes.h"

namespace meta {

Function::Function(utils::array_view<StackFrame> reduction):
    Visitable<Declaration, Function>(reduction),
    mChildren(getNodes(reduction))
{
    PRECONDITION(reduction.size() == 7 || reduction.size() == 8); // with or without annotations

    const bool hasAnnotations = reduction.size() == 8;
    if (hasAnnotations) {
        for (auto annotataion : reduction[0].nodes)
            std::invoke(&Annotation::setTarget, dynamic_cast<Annotation&>(*annotataion), this);
    }
    const size_t visibilityPos = (hasAnnotations ? 1 : 0);
    const size_t typePos = visibilityPos + 1;
    const size_t namePos = typePos + 1;
    const size_t argsPos = namePos + 2;
    auto visTokenIt = reduction[visibilityPos].tokens.begin();
    if (visTokenIt != reduction[visibilityPos].tokens.end())
        mVisibility = fromToken(*visTokenIt);
    mRetType = reduction[typePos].tokens;
    mName = reduction[namePos].tokens;
    for (auto argNode : reduction[argsPos].nodes) {
        auto& arg = dynamic_cast<VarDecl&>(*argNode);
        arg.flags() |= VarFlags::argument;
    }
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

CodeBlock* Function::body() {
    auto res = std::find_if(mChildren.rbegin(), mChildren.rend(), [](Node *node) {
        return node->getVisitableType() == std::type_index(typeid(CodeBlock));
    });
    return res == mChildren.rend() ? nullptr : static_cast<CodeBlock*>(res->get());
}

bool Function::is(Function::Attribute attr) const
{
    return (mAttributes & attr) != 0;
}

void Function::set(Function::Attribute attr, bool val)
{
    mAttributes = val ? (mAttributes | attr) : (mAttributes & ~attr);
}

const Declaration::AttributesMap Function::attrMap = {
    {"entrypoint", [](Declaration *decl) {
        std::invoke(&Function::set, dynamic_cast<Function&>(*decl), entrypoint, true);
    }}
};

} // namespace meta

