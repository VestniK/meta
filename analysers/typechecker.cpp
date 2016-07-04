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

#include "parser/metanodes.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

namespace meta::analysers {

using Type = const typesystem::Type*;
using Types = typesystem::TypesStore;

struct TypeEvaluator {
    Type operator() (Node* node, Types&) {throw UnexpectedNode(node, "Can't evaluate type");}

    Type operator() (Number* node, Types& types) {
        node->setType(types.getPrimitive(typesystem::Type::Int));
        return node->type();
    }

    Type operator() (Literal* node, Types& types) {
        switch (node->value()) {
        case Literal::trueVal:
        case Literal::falseVal:
            node->setType(types.getPrimitive(typesystem::Type::Bool));
        break;
        }
        return node->type();
    }

    Type operator() (StrLiteral* node, Types& types) {
        node->setType(types.getPrimitive(typesystem::Type::String));
        return node->type();
    }

    Type operator() (Var* node, Types&) {
        PRECONDITION(node->declaration() != nullptr);
        PRECONDITION(node->declaration()->type() != nullptr);
        PRECONDITION(node->declaration()->type()->typeId() != typesystem::Type::Auto);

        return node->declaration()->type();
    }

    Type operator() (Assigment* node, Types& types) {
        Type valueType = dispatch(*this, node->value(), types);

        struct {
            Type operator() (Node* node, Type) {throw UnexpectedNode(node, "Variable or access to Memeber required");}

            Type operator() (Var* node, Type valType) {
                if (node->declaration()->type() != valType)
                    throw SemanticError(
                        node, "Attempt to assign value of type '%s' to a the variable '%s' of type '%s'",
                        valType->name(), node->declaration()->name(), node->declaration()->type()->name()
                    );
                return node->declaration()->type();
            }
            Type operator() (MemberAccess* node, Type valType) {
                if (node->memberDecl()->type() != valType)
                    throw SemanticError(
                        node, "Attempt to assign value of type '%s' to a the variable '%s.%s.%s' of type '%s'",
                        valType->name(),
                        node->targetStruct()->package(), node->targetStruct()->name(),
                        node->memberDecl()->name(),
                        node->memberDecl()->type()->name()
                    );
                return node->memberDecl()->type();
            }
        } getAssignType;
        return dispatch(getAssignType, node->target(), valueType);
    }

    Type operator() (PrefixOp* node, Types& types) {
        Type operandType = dispatch(*this, node->operand(), types);
        switch (node->operation()) {
            case PrefixOp::positive:
            case PrefixOp::negative:
                if (!(operandType & typesystem::TypeProp::numeric))
                    throw SemanticError(
                        node,
                        "Can't perform arythmetic operation on value of type '%s'", operandType->name()
                    );
                break;
            case PrefixOp::boolnot:
                if (!(operandType & typesystem::TypeProp::boolean))
                    throw SemanticError(
                        node,
                        "Can't perform boolean not operation on value of type '%s'", operandType->name()
                    );
                break;
        }
        node->setType(operandType);
        return operandType;
    }

    Type operator() (BinaryOp* node, Types& types) {
        POSTCONDITION(node->type() != nullptr);
        Type lhs = dispatch(*this, node->left(), types);
        Type rhs = dispatch(*this, node->right(), types);

        switch (node->operation()) {
            case BinaryOp::add:
            case BinaryOp::div:
            case BinaryOp::sub:
            case BinaryOp::mul:
                if (
                    !(lhs & typesystem::TypeProp::numeric) ||
                    !(rhs & typesystem::TypeProp::numeric)
                )
                    throw SemanticError(
                        node, "Can't perform arythmetic operation on values of types '%s' and '%s'",
                        lhs->name(), rhs->name()
                    );
                /// @todo should return widest type of left or right. Now there is only one numeric
                // type so it's ok to return first arg type
                node->setType(lhs);
                break;

            case BinaryOp::equal:
            case BinaryOp::noteq:
                if (lhs != rhs)
                    throw SemanticError(node, "Can't compare values of types '%s' and '%s'", lhs->name(), rhs->name());
                node->setType(types.getPrimitive(typesystem::Type::Bool));
                break;
            case BinaryOp::greater:
            case BinaryOp::greatereq:
            case BinaryOp::less:
            case BinaryOp::lesseq:
                if (
                    !(lhs & typesystem::TypeProp::numeric) ||
                    !(rhs & typesystem::TypeProp::numeric)
                )
                    throw SemanticError(node, "Can't compare values of types '%s' and '%s'", lhs->name(), rhs->name());
                node->setType(types.getPrimitive(typesystem::Type::Bool));
                break;

            case BinaryOp::boolAnd:
            case BinaryOp::boolOr:
                if (
                    !(lhs & typesystem::TypeProp::boolean) ||
                    !(rhs & typesystem::TypeProp::boolean)
                )
                    throw SemanticError(
                        node, "Can't perform boolean operations on values of types '%s' and '%s'",
                        lhs->name(), rhs->name()
                    );
                node->setType(types.getPrimitive(typesystem::Type::Bool));
                break;
        }
        return node->type();
    }

    // Requires TypeChecker as complete type so definition is bellow
    Type operator() (Call* node, Types &types);
};

class TypeChecker: public Visitor {
public:
    explicit TypeChecker(typesystem::TypesStore &types): mTypes(types) {}

    bool visit(Function* node) override {
        if (node->type() != nullptr)
            return false;
        node->setType(mTypes.getByName(node->retType()));
        if (node->type() == nullptr)
            throw SemanticError(node, "Function '%s' returns unknown type '%s'", node->name(), node->retType());
        mCurrFunc = node;
        return true;
    }
    void leave(Function* node) override {
        POSTCONDITION(node->type() != nullptr);
        POSTCONDITION(node->type()->typeId() != typesystem::Type::Auto);

        mCurrFunc = nullptr;
    }

    bool visit(meta::ExprStatement* node) override {
        dispatch(TypeEvaluator{}, node->expression(), mTypes);
        return false;
    }

    bool visit(VarDecl* node) override {
        POSTCONDITION(node->type() != nullptr);
        POSTCONDITION(node->type()->typeId() != typesystem::Type::Auto);

        node->setType(mTypes.getByName(node->typeName()));
        if (node->type() == nullptr)
            throw SemanticError(node, "Variable '%s' has unknown type '%s'", node->name(), node->typeName());
        if (!node->inited()) {
            if (node->type()->typeId() == typesystem::Type::Auto)
                throw SemanticError(node, "Can't deduce variable '%s' type.", node->name());
            return false;
        }

        Type initExprType = dispatch(TypeEvaluator{}, node->initExpr(), mTypes);
        if (node->type()->typeId() == typesystem::Type::Auto)
            node->setType(initExprType);
        else if (node->type() != initExprType)
            throw SemanticError(
                node, "Attempt to init variable '%s' of type '%s' with value of type '%s'",
                node->name(), node->type()->name(), initExprType->name()
            );
        return false;
    }

    bool visit(If* node) override {
        Type condType = dispatch(TypeEvaluator{}, node->condition(), mTypes);
        if (!(condType & typesystem::TypeProp::boolean))
            throw SemanticError(node->condition(), "If statement can't work with condition of type '%s'", condType->name());
        if (node->thenBlock())
            node->thenBlock()->walk(this);
        if (node->elseBlock())
            node->elseBlock()->walk(this);
        return false;
    }

    bool visit(meta::Return* node) override {
        Type ret = node->value() == nullptr ?
            mTypes.getVoid():
            dispatch(TypeEvaluator{}, node->value(), mTypes)
        ;
        if (!(mCurrFunc->type() & typesystem::TypeProp::complete)) {
            if (ret->typeId() == typesystem::Type::Auto)
                throw SemanticError(node, "Can't return value of incomplete type");
            mCurrFunc->setType(ret);
        }
        else if (mCurrFunc->type() != ret)
            throw SemanticError(
                node, "Attempt to return value of type '%s' from function returning '%s'",
                ret->name(), mCurrFunc->type()->name()
            );
        return false;
    }

private:
    typesystem::TypesStore &mTypes;
    Function *mCurrFunc = nullptr;
};

Type TypeEvaluator::operator() (Call* node, Types &types) {
    PRECONDITION(node->args().size() == node->function()->args().size());
    if (node->function()->type() == nullptr) {
        TypeChecker subchecker(types);
        node->function()->walk(&subchecker);
    }
    const auto& argdecls = node->function()->args();
    const auto& args = node->args();
    for (size_t i = 0; i < args.size(); ++i) {
        Type argtype = dispatch(*this, args[i], types);
        if (argdecls[i]->type() != argtype)
            throw SemanticError(
                args[i], "Can't call function '%s' with argument %u of type '%s'. Expected type is '%s'",
                node->function()->name(), i, argtype->name(), argdecls[i]->type()->name()
            );
    }
    return node->function()->type();
}

void checkTypes(AST *ast, typesystem::TypesStore &types) {
    TypeChecker visitor(types);
    ast->walk(&visitor);
}

} // namespace meta::analysers
