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

#ifndef GENERATORS_TRANSLATION_RUNNER_H
#define GENERATORS_TRANSLATION_RUNNER_H

#include <cassert>
#include <stack>
#include <vector>

#include "parser/metaparser.h"
#include "parser/vardecl.h"

#include "generators/translator.h"

namespace generators {

template<typename Value>
class TranslationRunner: private meta::Visitor
{
public:
    void translate(meta::AST *ast, Translator<Value> *translator)
    {
        this->translator = translator;
        ast->walk(this);
        this->translator = nullptr;
    }

private:
    virtual void visit(meta::Function *node) override
    {
        translator->startFunction(node);
    }

    virtual void visit(meta::Call *) override
    {
        stack.push(nullptr);
    }

    virtual void leave(meta::Call *node) override
    {
        std::vector<Value> args;
        for (; stack.top() != nullptr; stack.pop())
            args.push_back(stack.top());
        stack.top() = translator->call(node, args);
    }

    virtual void visit(meta::Number *node) override
    {
        stack.push(translator->number(node));
    }

    virtual void visit(meta::Var *node) override
    {
        stack.push(translator->var(node));
    }

    virtual void leave(meta::VarDecl *node) override
    {
        assert(stack.size() == node->inited() ? 1 : 0);
        translator->declareVar(node, node->inited() ? stack.top() : nullptr);
        if (node->inited())
            stack.pop();
    }

    virtual void leave(meta::Assigment *node) override
    {
        assert(stack.size() == 1);
        stack.top() = translator->assign(node, stack.top());
    }

    virtual void leave(meta::BinaryOp *node) override
    {
        assert(stack.size() >=2);
        Value right = stack.top();
        stack.pop();
        Value left = stack.top();
        stack.top() = translator->binaryOp(node, left, right);
    }

    virtual void leave(meta::PrefixOp *node) override
    {
        assert(stack.size() > 0);
        stack.top() = translator->prefixOp(node, stack.top());
    }

    virtual void leave(meta::Return *node) override
    {
        assert(stack.size() == 1);
        translator->returnValue(node, stack.top());
        stack.pop();
    }
    virtual void leave (meta::ExprStatement *) override
    {
        assert(stack.size() == 1);
        stack.pop();
    }

private:
    std::stack<Value> stack;
    Translator<Value> *translator;
};

} // namespace generators

#endif // GENERATORS_TRANSLATION_RUNNER_H
