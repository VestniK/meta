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

Function::Function(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
    Visitable<Declaration, Function>(src, reduction)
{
    // 7: {opt{<visibility>}, <rettype>, <name>, '(', <arglist>, ')', opt{<body>}}
    // 8: {<annotations>, opt{<visibility>}, <rettype>, <name>, '(', <arglist>, ')', opt{<body>}}
    PRECONDITION(reduction.size() == 7 || reduction.size() == 8); // with or without annotations
    PRECONDITION(reduction[reduction.size() - 1].nodes.size() <= 1); // only one or no function body
    POSTCONDITION(mBody != nullptr || reduction[reduction.size() - 1].nodes.empty());
    POSTCONDITION(countNodes(reduction) == mAnnotations.size() + mArgs.size() + (mBody ? 1 : 0));
    utils::array_view<StackFrame> funcReduction = reduction;
    if (reduction.size() == 8) {
        for (auto& node: reduction[0].nodes) {
            auto& annotation = dynamic_cast<Annotation&>(*node);
            annotation.setTarget(this);
            mAnnotations.push_back(&annotation);
        }
        funcReduction = {reduction.data() + 1, reduction.size() - 1};
    }
    constexpr size_t visibilityPos = 0;
    constexpr size_t typePos = visibilityPos + 1;
    constexpr size_t namePos = typePos + 1;
    constexpr size_t argsPos = namePos + 2;
    constexpr size_t bodyPos = argsPos + 2;
    if (!funcReduction[visibilityPos].tokens.empty()) {
        const Token visTok = *funcReduction[visibilityPos].tokens.begin();
        mVisibility = fromToken(visTok);
    }
    mRetType = funcReduction[typePos].tokens;
    mName = funcReduction[namePos].tokens;
    for (auto argNode : funcReduction[argsPos].nodes) {
        auto& arg = dynamic_cast<VarDecl&>(*argNode);
        arg.flags() |= VarFlags::argument;
        mArgs.emplace_back(&arg);
    }
    if (!funcReduction[bodyPos].nodes.empty())
        mBody = &(dynamic_cast<CodeBlock&>(*funcReduction[bodyPos].nodes[0]));
}

Function::~Function() = default;

const Declaration::AttributesMap Function::attrMap = {
    {"entrypoint", [](Declaration *decl) {
        dynamic_cast<Function&>(*decl).flags() |= FuncFlags::entrypoint;
    }}
};

} // namespace meta

