/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include "parser/struct.h"

namespace meta {

Struct::Struct(utils::array_view<StackFrame> reduction):
    Visitable<Declaration, Struct>(reduction)
{
    // 6: {opt{<visibility>}, 'struct', <name>, '{', <memberslist>, '}'}
    // 7: {<annotations>, opt{<visibility>}, 'struct', <name>, '{', <memberslist>, '}'}
    PRECONDITION(reduction.size() == 6 || reduction.size() == 7);
    POSTCONDITION(countNodes(reduction) == mAnnotations.size() + mMembers.size());
    utils::array_view<StackFrame> structReduction = reduction;
    if (reduction.size() == 7) {
        for (auto& node: reduction[0].nodes) {
            auto& ann = dynamic_cast<Annotation&>(*node);
            ann.setTarget(this);
            mAnnotations.emplace_back(&ann);
        }
        structReduction = reduction.slice(1);
    }
    if (!structReduction[0].tokens.empty())
        mVisibility = fromToken(*structReduction[0].tokens.begin());
    mName = structReduction[2].tokens;
    for (auto& node: structReduction[4].nodes) {
        auto& member = dynamic_cast<VarDecl&>(*node);
        member.flags() |= VarFlags::member;
        mMembers.emplace_back(&member);
    }
}

const Declaration::AttributesMap Struct::attrMap = {};

}
