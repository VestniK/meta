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
#include "utils/slice.h"

#include "parser/function.h"
#include "parser/sourcefile.h"
#include "parser/struct.h"

#include "analysers/declconflicts.h"
#include "analysers/actions.h"
#include "analysers/semanticerror.h"

namespace meta::analysers {

void Actions::changeVisibility(utils::array_view<StackFrame> reduction) {
    PRECONDITION(reduction.size() == 2);
    PRECONDITION(reduction[0].tokens.begin() != reduction[0].tokens.end());
    const auto token = *(reduction[0].tokens.begin());
    mDefaultVisibility = fromToken(token);
}

void Actions::onFunction(Function* node) {
    PRECONDITION(!mCurrentPackage.empty());
    node->setPackage(mCurrentPackage);
    if (node->visibility() == Visibility::Default)
        node->setVisibility(mDefaultVisibility);
    auto& pkgDict = mDictionary[mCurrentPackage];
    auto structIt = pkgDict.structs.find(node->name());
    if (structIt != pkgDict.structs.end())
        throwDeclConflict(node, utils::slice(structIt));
    pkgDict.functions.emplace(node->name(), node);
}

void Actions::onSourceFile(SourceFile* node) {
    PRECONDITION(!mCurrentPackage.empty());
    node->setPackage(mCurrentPackage);
}

void Actions::onStruct(Struct* node) {
    PRECONDITION(!mCurrentPackage.empty());
    node->setPackage(mCurrentPackage);
    if (node->visibility() == Visibility::Default)
        node->setVisibility(mDefaultVisibility);
    /// @todo check and throw SemanticError if there is a function with the same name in the same package
    auto& pkgDict = mDictionary[mCurrentPackage];
    auto funcsRng = utils::slice(pkgDict.functions.equal_range(node->name()));
    if (!funcsRng.empty())
        throwDeclConflict(node, funcsRng);
    const auto res = pkgDict.structs.emplace(node->name(), node);
    if (!res.second)
        throwDeclConflict(node, utils::slice(res.first));
}

} // namespace meta
