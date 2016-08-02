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

#include <experimental/string_view>

#include "parser/metaparser.h"

namespace meta {

class Import: public Visitable<Node, Import>
{
public:
    Import(utils::array_view<StackFrame> reduction);

    const utils::string_view &name() const {return mName;}
    const utils::string_view &targetPackage() const {return mPackage;}
    const utils::string_view &target() const {return mTarget;}

    void walk(Visitor* visitor, int depth) override {
        accept(visitor);
        seeOff(visitor);
    }

private:
    std::vector<Node::Ptr<Node>> mChildren;
    utils::string_view mPackage;
    utils::string_view mTarget;
    utils::string_view mName;
};

} // namespace meta
