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

#include "utils/types.h"
#include "utils/bitmask.h"

#include "parser/expression.h"
#include "parser/metaparser.h"
#include "parser/typed.h"

namespace meta {

enum class VarFlags {
    argument,
    member
};

class VarDecl: public Visitable<Node, VarDecl>, public Typed {
public:
    VarDecl(utils::array_view<StackFrame> reduction);

    const utils::string_view& name() const {return mName;}

    const utils::string_view& typeName() const {return mTypeName;}

    bool inited() const {return mInitExpr != nullptr;}
    Expression* initExpr() const {return mInitExpr;}
    auto flags() const {return mFlags;}
    auto& flags() {return mFlags;}

    void walk(Visitor* visitor, int depth) override {
        if (accept(visitor) && depth != 0) {
            if (mInitExpr)
                mInitExpr->walk(visitor, depth - 1);
        }
        seeOff(visitor);
    }

private:
    Node::Ptr<Expression> mInitExpr;
    utils::string_view mName, mTypeName;
    utils::Bitmask<VarFlags> mFlags;
};

}
