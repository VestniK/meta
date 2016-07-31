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

#include <string>

#include "parser/expression.h"

namespace meta {

class Call: public Visitable<Expression, Call> {
public:
    Call(utils::array_view<StackFrame> reduction);

    utils::string_view functionName() const {return mFunctionName;}
    Function* function() const {return mFunction;}
    void setFunction(Function* func);

    const std::vector<Node::Ptr<Expression>>& args() const {return mArgs;}

    void walk(Visitor* visitor, int depth) override {
        if (accept(visitor) && depth != 0) {
            for (auto arg: mArgs)
                arg->walk(visitor, depth - 1);
        }
        seeOff(visitor);
    }

private:
    std::vector<Node::Ptr<Expression>> mArgs;
    utils::string_view mFunctionName;
    Function* mFunction = nullptr;
};

} // namespace meta
