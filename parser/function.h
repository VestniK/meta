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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

#include "parser/metaparser.h"
#include "parser/typed.h"

namespace meta {

class Function: public Node, public Typed
{
meta_NODE
public:
    Function(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &retType() const {return mRetType;}
    const std::string &package() const {return mPackage;}
    void setPackage(const std::string &pkg) {mPackage = pkg;}
    std::vector<VarDecl*> args();
private:
    std::string mPackage;
    std::string mName;
    std::string mRetType;
};

} // namespace meta

#endif // FUNCTION_H

