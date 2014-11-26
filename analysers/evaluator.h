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

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <cassert>
#include <stack>
#include <vector>

#include "utils/contract.h"

#include "parser/metanodes.h"
#include "parser/metaparser.h"

namespace analysers {

template<typename Value>
class Evaluator: public meta::Visitor
{
public:
    // Value consumers
    virtual void returnValue(meta::Return *node, Value val) = 0;
    virtual void returnVoid(meta::Return *node) = 0;
    virtual void varInit(meta::VarDecl *node, Value val) = 0;
    virtual void ifCond(meta::If *node, Value val) = 0;
    // Value providers
    virtual Value number(meta::Number *node) = 0;
    virtual Value literal(meta::Literal *node) = 0;
    virtual Value var(meta::Var *node) = 0;
    // Operations on values
    virtual Value call(meta::Call *node, const std::vector<Value> &args) = 0;
    virtual Value assign(meta::Assigment *node, Value val) = 0;
    virtual Value binaryOp(meta::BinaryOp *node, Value left, Value right) = 0;
    virtual Value prefixOp(meta::PrefixOp *node, Value val) = 0;

    // Visitor implementation
    virtual void leave(meta::Number *node) override {mStack.top().push_back(number(node));}
    virtual void leave(meta::Literal *node) override {mStack.top().push_back(literal(node));}
    virtual void leave(meta::Var *node) override {mStack.top().push_back(var(node));}
    virtual bool visit(meta::Return *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::Return *node) override
    {
        PRECONDITION(mStack.top().empty() || mStack.top().size() == 1);
        POSTCONDITION(mStack.empty()); /// @todo unit tests needed to check that all statements cunsume whole evaluation stack

        if (mStack.top().empty())
            returnVoid(node);
        else
            returnValue(node, mStack.top()[0]);
        mStack.pop();
    }
    virtual bool visit(meta::If *node) override
    {
        mStack.push(std::vector<Value>());
        node->condition()->walk(this);
        assert(mStack.top().size() == 1);
        auto val = mStack.top()[0];
        mStack.pop(); // clean stack to be able to assert stack corruption in other statement visit/leave functions
        ifCond(node, val);
        assert(mStack.empty()); /// @todo unit tests needed to check that all statements cunsume whole evaluation stack
        return false; // do not evaluate both then and else branches they must be evaluated in subclass ifCond implementation
    }
    virtual bool visit(meta::VarDecl *node) override
    {
        if (node->inited())
            mStack.push(std::vector<Value>());
        return true;
    }
    virtual void leave(meta::VarDecl *node) override
    {
        if (!node->inited())
            return;
        assert(mStack.top().size() == 1);
        varInit(node, mStack.top()[0]);
        mStack.pop();
        assert(mStack.empty()); /// @todo unit tests needed to check that all statements cunsume whole evaluation stack
    }
    virtual bool visit(meta::Call *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::Call *node) override
    {
        auto res = call(node, mStack.top());
        mStack.pop();
        mStack.top().push_back(res);
    }
    virtual bool visit(meta::Assigment *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::Assigment *node) override
    {
        assert(mStack.top().size() == 1);
        auto res = assign(node, mStack.top()[0]);
        mStack.pop();
        mStack.top().push_back(res);
    }
    virtual bool visit(meta::PrefixOp *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::PrefixOp *node) override
    {
        assert(mStack.top().size() == 1);
        auto res = prefixOp(node, mStack.top()[0]);
        mStack.pop();
        mStack.top().push_back(res);
    }
    virtual bool visit(meta::BinaryOp *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::BinaryOp *node) override
    {
        assert(mStack.top().size() == 2);
        auto res = binaryOp(node, mStack.top()[0], mStack.top()[1]);
        mStack.pop();
        mStack.top().push_back(res);
    }
    virtual bool visit(meta::ExprStatement *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::ExprStatement *) override
    {
        mStack.pop();
        assert(mStack.empty()); /// @todo unit tests needed to check that all statements cunsume whole evaluation stack
    }

private:
    std::stack< std::vector<Value> > mStack;
};

} // namespace analysers

#endif // EVALUATOR_H
