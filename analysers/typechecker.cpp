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

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

#include "analysers/evaluator.h"
#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

namespace analysers {

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
    virtual const typesystem::Type *literal(meta::Literal *node) override
    {
        switch (node->value()) {
            case meta::Literal::trueVal:
            case meta::Literal::falseVal: node->setType(mTypes.getPrimitive(typesystem::Type::Bool)); break;
        }
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

void checkTypes(meta::AST *ast, typesystem::TypesStore &types)
{
    TypeChecker visitor(types);
    ast->walk(&visitor);
}

} // namespace analysers
