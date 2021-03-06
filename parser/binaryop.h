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

#include "parser/expression.h"

namespace meta {

class BinaryOp: public Visitable<Expression, BinaryOp> {
public:
    BinaryOp(const utils::SourceFile& src, utils::array_view<StackFrame> reduction);

    Expression* left() {return mLeft;}
    Expression* right() {return mRight;}

    enum Operation {
        // Arythmetic
        add, sub, mul, div,
        // Comparisions
        equal, noteq, less, greater, lesseq, greatereq,
        // Boolean operations
        boolAnd, boolOr
    };
    Operation operation() const {return mOp;}

    void walk(Visitor* visitor, int depth) override {
        if (accept(visitor) && depth != 0) {
            mLeft->walk(visitor, depth - 1);
            mRight->walk(visitor, depth - 1);
        }
        this->seeOff(visitor);
    }

private:
    Operation mOp;
    Node::Ptr<Expression> mLeft;
    Node::Ptr<Expression> mRight;
};

} // namespace meta
