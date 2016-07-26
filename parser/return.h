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

#include "utils/contract.h"

#include "parser/metaparser.h"

namespace meta {

class Return: public Visitable<Node, Return> {
public:
    Return(utils::array_view<StackFrame> reduction):
        Visitable<Node, Return>(reduction)
    {
        PRECONDITION(reduction.size() == 3);
        PRECONDITION(reduction[1].nodes.size() <= 1);
        POSTCONDITION(reduction[1].nodes.empty() || mRetVal != nullptr);
        if (reduction[1].nodes.empty())
            return;
        mRetVal = dynamic_cast<Expression*>(reduction[1].nodes[0].get());
    }

    Expression* value() {
        return mRetVal;
    }

    void walk(Visitor* visitor, int depth) override {
        if (accept(visitor) && depth != 0 && mRetVal)
            mRetVal->walk(visitor, depth - 1);
        seeOff(visitor);
    }

private:
    Node::Ptr<Expression> mRetVal;
};

}
