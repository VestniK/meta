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

#include <stack>
#include <vector>

#include "utils/contract.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

#include "analysers/evaluator.h"
#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

namespace meta {
namespace analysers {

class TypeChecker: public Evaluator<const typesystem::Type*>
{
public:
    explicit TypeChecker(typesystem::TypesStore &types): mTypes(types) {}

    virtual bool visit(Function *node)
    {
        if (node->type() != nullptr)
            return false;
        node->setType(mTypes.getByName(node->retType()));
        if (node->type() == nullptr)
            throw analysers::SemanticError(
                node, "Function '%*.s' returns unknown type '%*.s'",
                static_cast<int>(node->name().size()), node->name().data(),
                static_cast<int>(node->retType().size()), node->retType().data()
            );
        mCurrFunc = node;
        return true;
    }
    virtual void leave(Function *node) override
    {
        POSTCONDITION(node->type() != nullptr);
        POSTCONDITION(node->type()->typeId() != typesystem::Type::Auto);

        mCurrFunc = nullptr;
    }

    virtual const typesystem::Type *number(Number *node) override
    {
        node->setType(mTypes.getPrimitive(typesystem::Type::Int));
        return node->type();
    }
    virtual const typesystem::Type *literal(Literal *node) override
    {
        switch (node->value()) {
            case Literal::trueVal:
            case Literal::falseVal: node->setType(mTypes.getPrimitive(typesystem::Type::Bool)); break;
        }
        return node->type();
    }
    virtual const typesystem::Type *strLiteral(StrLiteral *node) override
    {
        node->setType(mTypes.getPrimitive(typesystem::Type::String));
        return node->type();
    }
    virtual const typesystem::Type *var(Var *node) override
    {
        PRECONDITION(node->declaration() != nullptr);
        PRECONDITION(node->declaration()->type() != nullptr);
        PRECONDITION(node->declaration()->type()->typeId() != typesystem::Type::Auto);

        return node->declaration()->type();
    }

    virtual void leave(VarDecl *node) override
    {
        POSTCONDITION(node->type() != nullptr);
        POSTCONDITION(node->type()->typeId() != typesystem::Type::Auto);

        node->setType(mTypes.getByName(node->typeName()));
        if (node->type() == nullptr)
            throw analysers::SemanticError(
                node, "Variable '%*.s' has unknown type '%*.s'",
                static_cast<int>(node->name().size()), node->name().data(),
                static_cast<int>(node->typeName().size()), node->typeName().data()
            );
        if (node->inited())
            return Evaluator<const typesystem::Type*>::leave(node);
        if (node->type()->typeId() == typesystem::Type::Auto)
            throw analysers::SemanticError(
                node, "Can't deduce variable '%*.s' type.",
                static_cast<int>(node->name().size()), node->name().data()
            );
    }
    virtual void varInit(VarDecl *node, const typesystem::Type *val) override
    {
        if (node->type()->typeId() == typesystem::Type::Auto) {
            node->setType(val);
            return;
        }
        if (node->type() != val) // TODO: implicit type conversations here
            throw analysers::SemanticError(
                node, "Attempt to init variable '%*.s' of type '%*.s' with value of type '%*.s'",
                static_cast<int>(node->name().size()), node->name().data(),
                static_cast<int>(node->type()->name().size()), node->type()->name().data(),
                static_cast<int>(val->name().size()), val->name().data()
            );
    }

    virtual void ifCond(If *node, const typesystem::Type *val) override
    {
        if (!val->is(typesystem::Type::boolean))
            throw SemanticError(
                node->condition(), "If statement can't work with condition of type '%*.s'",
                static_cast<int>(val->name().size()), val->name().data()
            );
        if (node->thenBlock())
            node->thenBlock()->walk(this);
        if (node->elseBlock())
            node->elseBlock()->walk(this);
    }

    virtual void returnValue(Return *node, const typesystem::Type *val) override
    {
        if (mCurrFunc->type()->typeId() == typesystem::Type::Auto) {
            if (val->typeId() == typesystem::Type::Auto)
                throw analysers::SemanticError(node, "Can't return value of incomplete type");
            mCurrFunc->setType(val);
            return;
        }
        if (mCurrFunc->type() != val)
            throw analysers::SemanticError(
                node, "Attempt to return value of type '%*.s' from function returning '%*.s'",
                static_cast<int>(val->name().size()), val->name().data(),
                static_cast<int>(mCurrFunc->type()->name().size()), mCurrFunc->type()->name().data()
            );
    }

    virtual void returnVoid(Return *node) override
    {
        returnValue(node, mTypes.getVoid());
    }

    virtual const typesystem::Type *assign(Assigment *node, const typesystem::Type *val) override
    {
        if (node->declaration()->type() != val) // TODO: implicit type conversations here
            throw analysers::SemanticError(
                node, "Attempt to assign value of type '%*.s' to a the variable '%*.s' of type '%*.s'",
                static_cast<int>(val->name().size()), val->name().data(),
                static_cast<int>(node->varName().size()), node->varName().data(),
                static_cast<int>(node->declaration()->type()->name().size()), node->declaration()->type()->name().data()
            );
        return node->declaration()->type();
    }

    virtual const typesystem::Type *prefixOp(PrefixOp *node, const typesystem::Type *val) override
    {
        switch (node->operation()) {
            case PrefixOp::positive:
            case PrefixOp::negative:
                if (!val->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(
                        node, "Can't perform arythmetic operation on value of type '%*.s'",
                        static_cast<int>(val->name().size()), val->name().data()
                    );
                break;
            case PrefixOp::boolnot:
                if (!val->is(typesystem::Type::boolean))
                    throw analysers::SemanticError(
                        node, "Can't perform boolean not operation on value of type '%*.s'",
                        static_cast<int>(val->name().size()), val->name().data());
                break;
        }
        node->setType(val);
        return val;
    }

    virtual const typesystem::Type *binaryOp(BinaryOp *node, const typesystem::Type *left, const typesystem::Type *right) override
    {
        POSTCONDITION(node->type() != nullptr);

        switch (node->operation()) {
            case BinaryOp::add:
            case BinaryOp::div:
            case BinaryOp::sub:
            case BinaryOp::mul:
                if (!left->is(typesystem::Type::numeric) || !right->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(
                        node, "Can't perform arythmetic operation on values of types '%*.s' and '%*.s'",
                        static_cast<int>(left->name().size()), left->name().data(),
                        static_cast<int>(right->name().size()), right->name().data()
                    );
                node->setType(left); // TODO: should return widest type of left or right. Now there is only one numeric type so it's ok to return first arg type
                break;

            case BinaryOp::equal:
            case BinaryOp::noteq:
                if (left != right)
                    throw analysers::SemanticError(
                        node, "Can't compare values of types '%*.s' and '%*.s'",
                        static_cast<int>(left->name().size()), left->name().data(),
                        static_cast<int>(right->name().size()), right->name().data()
                    );
                node->setType(mTypes.getPrimitive(typesystem::Type::Bool));
                break;
            case BinaryOp::greater:
            case BinaryOp::greatereq:
            case BinaryOp::less:
            case BinaryOp::lesseq:
                if (!left->is(typesystem::Type::numeric) || !right->is(typesystem::Type::numeric))
                    throw analysers::SemanticError(
                        node, "Can't compare values of types '%*.s' and '%*.s'",
                        static_cast<int>(left->name().size()), left->name().data(),
                        static_cast<int>(right->name().size()), right->name().data()
                    );
                node->setType(mTypes.getPrimitive(typesystem::Type::Bool));
                break;

            case BinaryOp::boolAnd:
            case BinaryOp::boolOr:
                if (!left->is(typesystem::Type::boolean) || !right->is(typesystem::Type::boolean))
                    throw analysers::SemanticError(
                        node, "Can't perform boolean operations on values of types '%*.s' and '%*.s'",
                        static_cast<int>(left->name().size()), left->name().data(),
                        static_cast<int>(right->name().size()), right->name().data());
                node->setType(mTypes.getPrimitive(typesystem::Type::Bool));
                break;
        }
        return node->type();
    }

    virtual const typesystem::Type *call(Call *node, const std::vector<const typesystem::Type *> &args)
    {
        PRECONDITION(node->function()->args().size() == args.size());
        PRECONDITION(args.size() == node->args().size());

        if (node->function()->type() == nullptr) {
            TypeChecker subchecker(mTypes);
            node->function()->walk(&subchecker);
        }
        auto argdecls = node->function()->args();
        for (size_t i = 0; i < args.size(); ++i) {
            if (argdecls[i]->type() != args[i])
                throw analysers::SemanticError(
                    node->args()[i], "Can't call function '%*.s' with argument %u of type '%*.s'. Expected type is '%*.s'",
                    static_cast<int>(node->function()->name().size()), node->function()->name().data(),
                    (unsigned)i,
                    static_cast<int>(args[i]->name().size()), args[i]->name().data(),
                    static_cast<int>(argdecls[i]->type()->name().size()), argdecls[i]->type()->name().data()
                );
        }
        return node->function()->type();
    }

private:
    typesystem::TypesStore &mTypes;
    Function *mCurrFunc = nullptr;
};

void checkTypes(AST *ast, typesystem::TypesStore &types)
{
    TypeChecker visitor(types);
    ast->walk(&visitor);
}

} // namespace analysers
} // namspace meta
