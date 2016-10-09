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

#include "parser/expression.h"

namespace meta {

class Var: public Visitable<Expression, Var> {
public:
    Var(const utils::SourceFile& src, utils::array_view<StackFrame> reduction);

    const utils::string_view& name() const {return mName;}
    VarDecl* declaration() {return mDeclaration;}
    void setDeclaration(VarDecl* decl) {mDeclaration = decl;}

    void walk(Visitor* visitor, int) override {
        accept(visitor);
        seeOff(visitor);
    }

private:
    utils::string_view mName;
    VarDecl* mDeclaration;
};

}
