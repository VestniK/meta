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
#pragma once

#include "utils/types.h"

#include "parser/declaration.h"
#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

class Struct: public Visitable<Declaration, Struct> {
public:
    Struct(utils::array_view<StackFrame> reduction);

    const Declaration::AttributesMap& attributes() const override {return attrMap;}

    utils::string_view name() const {return mName;}

    utils::string_view package() const {return mPackage;}
    void setPackage(utils::string_view pkg) {mPackage = pkg;}

    Visibility visibility() const {return mVisibility;}
    void setVisibility(Visibility val) {mVisibility = val;}

private:
    utils::string_view mName;
    utils::string_view mPackage;
    Visibility mVisibility = Visibility::Default;

    static const Declaration::AttributesMap attrMap;
};

}
