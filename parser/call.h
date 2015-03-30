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

class Call: public Expression
{
meta_NODE
public:
    Call(const StackFrame *reduction, size_t size);

    const std::string &functionName() const {return mFunctionName;}
    Function *function() const {return mFunction;}
    void setFunction(Function *func);

    Node *arg(size_t pos);
    size_t argsCount() const;

private:
    std::string mFunctionName;
    Function *mFunction;
};

} // namespace meta
