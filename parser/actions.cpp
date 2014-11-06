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

#include "parser/actions.h"
#include "parser/function.h"
#include "parser/sourcefile.h"

namespace meta {

void Actions::package(const meta::StackFrame *reduction, size_t size)
{
    assert(size == 3);
    mPackage = reduction[1].tokens;
}

void Actions::changeVisibility(const meta::StackFrame *reduction, size_t size)
{
    assert(size == 2);
    assert(reduction[0].tokens.begin() != reduction[0].tokens.end());
    auto token = *(reduction[0].tokens.begin());
    mDefaultVisibility = meta::fromToken(token);
}

void Actions::onFunction(meta::Function *node)
{
    node->setPackage(mPackage);
    if (node->visibility() == meta::Visibility::Default)
        node->setVisibility(mDefaultVisibility);
    mDictionary[mPackage].insert({node->name(), node});
}

void Actions::onSourceFile(SourceFile *node)
{
    node->setPackage(mPackage);
}

} // namespace meta
