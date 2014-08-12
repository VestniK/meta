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

#ifndef ASSIGMENT_H
#define ASSIGMENT_H

#include "parser/metaparser.h"

namespace meta {

class Assigment: public Node
{
meta_NODE
public:
    Assigment(AST *ast, const StackFrame* start, size_t size);

    const std::string &varName() const {return mVarName;}
    VarDecl *declaration() {return mDeclaration;}
    void setDeclaration(VarDecl *decl) {mDeclaration = decl;}

private:
    std::string mVarName;
    VarDecl *mDeclaration;
};

}

#endif // ASSIGMENT_H
