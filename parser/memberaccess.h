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

#include "utils/contract.h"

#include "parser/expression.h"

namespace meta {

class MemberAccess: public Visitable<Expression, MemberAccess>, public Typed
{
public:
    MemberAccess(utils::array_view<StackFrame> reduction):
        Visitable<Expression, MemberAccess>(reduction)
    {
        PRECONDITION(reduction.size() == 3);
        PRECONDITION(static_cast<utils::string_view>(reduction[1].tokens) == ".");
        PRECONDITION(reduction[0].nodes.size() == 1);
        PRECONDITION(reduction[2].nodes.size() == 0);
        PRECONDITION(reduction[2].symbol == Terminal::identifier);

        mAggregate = dynamic_cast<Expression*>(reduction[0].nodes.front());
        assert(mAggregate != nullptr);
        mMemberName = reduction[2].tokens;
    }

    Struct* targetStruct() const {return mTargetStruct;}
    void setTargetStruct(Struct* val) {mTargetStruct = val;}

    VarDecl* memberDecl() const {return mMemberDecl;}
    void setMemberDecl(VarDecl* val) {mMemberDecl = val;}

    utils::string_view memberName() const {return mMemberName;}

private:
    Expression* mAggregate;
    Struct* mTargetStruct = nullptr;
    VarDecl* mMemberDecl = nullptr;
    utils::string_view mMemberName;
};

} // namespace meta
