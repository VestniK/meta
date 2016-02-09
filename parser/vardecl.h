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
#include "parser/typed.h"

namespace meta {

class VarDecl: public Visitable<Node, VarDecl>, public Typed
{
public:
    VarDecl(const StackFrame* start, size_t size);

    enum Flags
    {
        argument = (1<<0)
    };

    const utils::string_view &name() const {return mName;}

    const utils::string_view &typeName() const {return mTypeName;}

    bool inited() const;
    Node *initExpr();
    bool is(Flags flag) const;
    void set(Flags flag, bool val = true);

private:
    utils::string_view mName, mTypeName;
    int mFlags;
};

}
