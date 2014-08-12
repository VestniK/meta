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

#ifndef GENERATORS_TRANSLATOR_H
#define GENERATORS_TRANSLATOR_H

namespace meta {

class Function;
class VarDecl;
class Return;
class Call;
class Number;
class Var;
class Assigment;
class BinaryOp;
class PrefixOp;

} // namespace meta

namespace generators {

template<typename Value>
class Translator
{
public:
    virtual ~Translator() {}

    virtual void startFunction(meta::Function *node) = 0;
    // Value consumers
    virtual void declareVar(meta::VarDecl *node, Value initialVal) = 0;
    virtual void returnValue(meta::Return *node, Value val) = 0;
    // Value providers
    virtual Value number(meta::Number *node) = 0;
    virtual Value var(meta::Var *node) = 0;
    // Operations on values
    virtual Value call(meta::Call *node, const std::vector<Value> &args) = 0;
    virtual Value assign(meta::Assigment *node, Value val) = 0;
    virtual Value binaryOp(meta::BinaryOp *node, Value left, Value right) = 0;
    virtual Value prefixOp(meta::PrefixOp *node, Value val) = 0;
};

} // namespace generators

#endif // TRANSLATOR_H
