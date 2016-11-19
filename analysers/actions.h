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
#pragma once

#include <deque>
#include <set>

#include "utils/types.h"

#include "parser/metaparser.h"
#include "parser/visibility.h"

#include "analysers/dictionary.h"

namespace meta::analysers {

class Actions: public ParseActions, public NodeActions {
public:
    void package(utils::array_view<StackFrame> reduction) override;
    void changeVisibility(utils::array_view<StackFrame> reduction) override;
    void onFunction(Function* node) override;
    void onSourceFile(SourceFile* node) override;
    void onStruct(Struct* node) override;

    Dictionary& dictionary() {return mDictionary;}

private:
    Visibility mDefaultVisibility = Visibility::Private;
    Dictionary mDictionary;
    utils::string_view mCurrentPackage;
};

} // namespace meta
