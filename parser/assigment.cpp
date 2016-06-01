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
#include "utils/contract.h"

#include "parser/assigment.h"
#include "parser/metanodes.h"

namespace meta {

Assigment::Assigment(utils::array_view<StackFrame> reduction):
    Visitable<Expression, Assigment>(reduction)
{
    PRECONDITION(reduction.size() == 3);
    PRECONDITION(reduction[0].nodes.size() == 1);
    PRECONDITION(reduction[1].nodes.empty());
    PRECONDITION(reduction[2].nodes.size() == 1);
}

Node* Assigment::value() {
    PRECONDITION(children.size() == 2);
    return children.back();
}

VarDecl* Assigment::targetDeclaration() {
    PRECONDITION(children.size() == 2);

    struct {

    VarDecl* operator() (Node*) {assert(false); return nullptr;} // TODO: throw UnexpectedNode here!!!

    VarDecl* operator() (Var* var) {return var->declaration();}
    VarDecl* operator() (MemberAccess* member) {return member->memberDecl();}

    } getVarDecl;

    return dispatch(getVarDecl, children.front());
}

utils::string_view Assigment::targetVarName() {
    PRECONDITION(children.size() == 2);

    struct {

    utils::string_view operator() (Node*) {assert(false); return {};} // TODO: throw UnexpectedNode here!!!

    utils::string_view operator() (Var* var) {return var->name();}
    utils::string_view operator() (MemberAccess* member) {return member->memberName();}

    } getVarName;

    return dispatch(getVarName, children.front());
}

}
