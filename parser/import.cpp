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

#include "parser/import.h"

namespace meta {

Import::Import(utils::array_view<StackFrame> reduction):
    Visitable<Node, Import>(reduction),
    mChildren(getNodes(reduction))
{
    PRECONDITION(reduction.size() == 3 || reduction.size() == 5);

    Token target;
    TokenSequence package;
    for (auto token : reduction[1].tokens) {
        if (token.termNum != identifier)
            continue;
        package.setLast(target);
        target = token;
    }
    mName = mTarget = target;
    mPackage = package;
    if (reduction.size() == 5)
        mName = reduction[3].tokens;
}

} // namespace meta

