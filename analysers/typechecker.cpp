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

#include <cassert>
#include <stack>
#include <vector>

#include "parser/assigment.h"
#include "parser/binaryop.h"
#include "parser/call.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/number.h"
#include "parser/prefixop.h"
#include "parser/return.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

namespace {

template<typename Value>
class Evaluator: public meta::Visitor
{
public:
    // Value consumers
    virtual void returnValue(meta::Return *node, Value val) = 0;
    virtual void varInit(meta::VarDecl *node, Value val) = 0;
    // Value providers
    virtual Value number(meta::Number *node) = 0;
    virtual Value var(meta::Var *node) = 0;
    // Operations on values
    virtual Value call(meta::Call *node, const std::vector<Value> &args) = 0;
    virtual Value assign(meta::Assigment *node, Value val) = 0;
    virtual Value binaryOp(meta::BinaryOp *node, Value left, Value right) = 0;
    virtual Value prefixOp(meta::PrefixOp *node, Value val) = 0;

    // Visitor implementation
    virtual void leave(meta::Number *node) override {mStack.top().push_back(number(node));}
    virtual void leave(meta::Var *node) override {mStack.top().push_back(var(node));}
    virtual bool visit(meta::Return *) override {mStack.push(std::vector<Value>()); return true;}
    virtual void leave(meta::Return *node) override
    {
        assert(mStack.top().size() == 1);
        returnValue(node, mStack.top()[0]);
        mStack.pop();
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
    virtual void leave(meta::ExprStatement *) override {mStack.pop();}

private:
    std::stack< std::vector<Value> > mStack;
};

class TypeChecker: public Evaluator<const typesystem::Type*>
{
public:
    explicit TypeChecker(typesystem::TypesStore &types): mTypes(types) {}

    virtual bool visit(meta::Function *node)
    {
        if (node->type() != nullptr)
            return false;
        node->setType(mTypes.getByName(node->retType()));
        if (node->type() == nullptr)
            throw analysers::SemanticError(node, "Function '%s' returns unknown type '%s'", node->name().c_str(), node->retType().c_str());
        mCurrFunc = node;
        return true;
    }
    virtual void leave(meta::Function *) override
    {
        mCurrFunc = nullptr;
    }

    virtual const typesystem::Type *number(meta::Number *node) override
    {
        node->setType(mTypes.getPrimitive(typesystem::Type::Int));
        return node->type();
    }
    virtual const typesystem::Type *var(meta::Var *node) override
    {
        assert(node->declaration()->type() != nullptr); // declaration of unknown type should rize error before var usage
        assert(node->declaration()->type()->typeId() != typesystem::Type::Auto); // unresolved auto should rize error on declaration
        return node->declaration()->type();
    }

    virtual void leave(meta::VarDecl *node) override
    {
        node->setType(mTypes.getByName(node->typeName()));
        if (node->type() == nullptr)
            throw analysers::SemanticError(node, "Variable '%s' has unknown type '%s'", node->name().c_str(), node->typeName().c_str());
        if (node->inited())
            return Evaluator<const typesystem::Type*>::leave(node);
        if (node->type()->typeId() == typesystem::Type::Auto)
            throw analysers::SemanticError(node, "Can't deduce variable '%s' type.", node->name().c_str());
    }
    virtual void varInit(meta::VarDecl *node, const typesystem::Type *val) override
    {
        if (node->type()->typeId() == typesystem::Type::Auto) {
            node->setType(val);
            return;
        }
        if (node->type() != val) // TODO: implicit type conversations here
            throw analysers::SemanticError(
                node, "Attempt to init variable '%s' of type '%s' with value of type '%s'",
                node->name().c_str(), node->type()->name().c_str(), val->name().c_str()
            );
    }

    virtual void returnValue(meta::Return *node, const typesystem::Type *val) override
    {
        if (mCurrFunc->type()->typeId() == typesystem::Type::Auto) {
            if (val->typeId() == typesystem::Type::Auto)
                throw analysers::SemanticError(node, "Can't return value of incomplete type");
            mCurrFunc->setType(val);
            return;
        }
        if (mCurrFunc->type() != val)
            throw analysers::SemanticError(node, "Attempt to return value of type '%s' from function returning '%s'", val->name().c_str(), mCurrFunc->type()->name().c_str());
    }

    virtual const typesystem::Type *assign(meta::Assigment *node, const typesystem::Type *val) override
    {
        if (node->declaration()->type() != val) // TODO: implicit type conversations here
            throw analysers::SemanticError(
                node, "Attempt to assign value of type '%s' to a the variable '%s' of type '%s'",
                val->name().c_str(), node->varName().c_str(), node->declaration()->type()->name().c_str()
            );
        return node->declaration()->type();
    }

    virtual const typesystem::Type *prefixOp(meta::PrefixOp *node, const typesystem::Type *val) override
    {
        switch (node->operation()) {
            case meta::PrefixOp::positive:
            case meta::PrefixOp::negative:
                if (!val->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(node, "Can't perform arythmetic operation on value of type '%s'", val->name().c_str());
        }
        node->setType(val);
        return val;
    }

    virtual const typesystem::Type *binaryOp(meta::BinaryOp *node, const typesystem::Type *left, const typesystem::Type *right) override
    {
        switch (node->operation()) {
            case meta::BinaryOp::add:
            case meta::BinaryOp::div:
            case meta::BinaryOp::sub:
            case meta::BinaryOp::mul:
                if (!left->is(typesystem::Type::numeric) || !right->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(node, "Can't perform arythmetic operation on values of types '%s' and '%s'", left->name().c_str(), right->name().c_str());
                node->setType(left);
                return node->type(); // TODO: should return widest type of left or right. Now there is only one numeric type so it's ok to return first arg type

            case meta::BinaryOp::equal:
            case meta::BinaryOp::noteq:
                if (left != right)
                    throw analysers::SemanticError(node, "Can't compare values of types '%s' and '%s'", left->name().c_str(), right->name().c_str());
                node->setType(mTypes.getPrimitive(typesystem::Type::Bool));
                return node->type();
            case meta::BinaryOp::greater:
            case meta::BinaryOp::greatereq:
            case meta::BinaryOp::less:
            case meta::BinaryOp::lesseq:
                if (!left->is(typesystem::Type::numeric) || !right->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(node, "Can't compare values of types '%s' and '%s'", left->name().c_str(), right->name().c_str());
                node->setType(mTypes.getPrimitive(typesystem::Type::Bool));
                return node->type();

            case meta::BinaryOp::operationsCount:
            default: break;
        }
        assert(false);
        return nullptr;
    }

    virtual const typesystem::Type *call(meta::Call *node, const std::vector<const typesystem::Type *> &args)
    {
        if (node->function()->type() == nullptr) {
            TypeChecker subchecker(mTypes);
            node->function()->walk(&subchecker);
        }
        auto argdecls = node->function()->args();
        assert(argdecls.size() == args.size() && args.size() == node->argsCount()); // Incorrect number of arguments should be checked during resolve phase
        for (size_t i = 0; i < args.size(); ++i) {
            if (argdecls[i]->type() != args[i])
                throw analysers::SemanticError(
                    node->arg(i), "Can't call function '%s' with argument %u of type '%s'. Expected type is '%s'",
                    node->function()->name().c_str(), (unsigned)i, args[i]->name().c_str(), argdecls[i]->type()->name().c_str()
                );
        }
        return node->function()->type();
    }

private:
    typesystem::TypesStore &mTypes;
    meta::Function *mCurrFunc = nullptr;
};

} // anonymous namespace

namespace analysers {

void checkTypes(meta::AST *ast, typesystem::TypesStore &types)
{
    TypeChecker visitor(types);
    ast->walk(&visitor);
}

} // namespace analysers
