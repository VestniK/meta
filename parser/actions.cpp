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
#include "parser/actions.h"
#include "parser/function.h"
#include "parser/sourcefile.h"

namespace meta {

void Actions::changeVisibility(const StackFrame *reduction, size_t size)
{
    PRECONDITION(size == 2);
    PRECONDITION(reduction[0].tokens.begin() != reduction[0].tokens.end());
    const auto token = *(reduction[0].tokens.begin());
    mDefaultVisibility = fromToken(token);
}

void Actions::onFunction(Function *node)
{
    PRECONDITION(!mCurrentPackage.empty());
    node->setPackage(mCurrentPackage);
    if (node->visibility() == Visibility::Default)
        node->setVisibility(mDefaultVisibility);
    mDictionary[mCurrentPackage].emplace(node->name(), node);
}

void Actions::onSourceFile(SourceFile *node)
{
    PRECONDITION(!mCurrentPackage.empty());
    node->setPackage(mCurrentPackage);
}

} // namespace meta
