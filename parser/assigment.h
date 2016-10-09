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
#include "utils/contract.h"

#include "parser/expression.h"

namespace meta {

class Assigment: public Visitable<Expression, Assigment> {
public:
    Assigment(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
        Visitable<Expression, Assigment>(src, reduction)
    {
        PRECONDITION(reduction.size() == 3);
        PRECONDITION(reduction[0].nodes.size() == 1);
        PRECONDITION(reduction[1].nodes.empty());
        PRECONDITION(reduction[2].nodes.size() == 1);

        POSTCONDITION(mTarget != nullptr);
        POSTCONDITION(mValue != nullptr);

        mTarget = dynamic_cast<Expression*>(reduction[0].nodes[0].get());
        mValue = dynamic_cast<Expression*>(reduction[2].nodes[0].get());
    }

    void walk(Visitor* visitor, int depth) override {
        if (accept(visitor) && depth != 0) {
            mTarget->walk(visitor, depth - 1);
            mValue->walk(visitor, depth - 1);
        }
        seeOff(visitor);
    }

    Expression* value() {return mValue;}
    Expression* target() {return mTarget;}

private:
    Node::Ptr<Expression> mTarget;
    Node::Ptr<Expression> mValue;
};

}
